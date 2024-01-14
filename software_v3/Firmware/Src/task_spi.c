/*
 * task_spi.c
 *
 *  Created on: Feb 6, 2023
 *      Author: Calvin
 */

#include "task_spi.h"
#include "task_main.h"
#include "task_counter.h"

#include "tim.h"
#include "spi.h"
#include "ads8668.h"
#include "max5548.h"

#include <stdint.h>
#include <stdbool.h>

volatile SPI1_STATE spi_state;

osThreadId_t th_tsk_SPI;
static uint64_t stk_tsk_SPI[32 * 4]; // 256*4
static StaticTask_t cb_tsk_SPI;
const osThreadAttr_t attr_tsk_SPI = { .name = "SPI_TASK", .stack_size =
		sizeof(stk_tsk_SPI), .stack_mem = stk_tsk_SPI, .cb_size =
		sizeof(cb_tsk_SPI), .cb_mem = &cb_tsk_SPI, .priority =
		(osPriority_t) osPriorityRealtime };
void taskSPI(void *argument);

osSemaphoreId_t s_adc_dma_cplt;
static StaticSemaphore_t cb_s_adc_dma_cplt;
static osSemaphoreAttr_t attr_s_adc_dma_cplt = { .cb_size =
		sizeof(cb_s_adc_dma_cplt), .cb_mem = &cb_s_adc_dma_cplt };

osMutexId_t mtx_spi1;
static StaticSemaphore_t cb_mtx_spi1;
const osMutexAttr_t attr_mtx_spi1 = { .cb_size = sizeof(cb_mtx_spi1), .cb_mem =
		&cb_mtx_spi1 };

osMutexId_t mtx_adc;
static StaticSemaphore_t cb_mtx_adc;
const osMutexAttr_t attr_mtx_adc = { .cb_size = sizeof(cb_mtx_adc), .cb_mem =
		&cb_mtx_adc, .attr_bits =
osMutexPrioInherit };

osMutexId_t mtx_isrc;
static StaticSemaphore_t cb_mtx_isrc;
const osMutexAttr_t attr_mtx_isrc = { .cb_size = sizeof(cb_mtx_isrc), .cb_mem =
		&cb_mtx_isrc, .attr_bits =
osMutexPrioInherit };

// ADC variables
adc_range_t adc_ranges[ADS8668_NUM_CHANNELS];

uint32_t adc_readings_accumulator[ADS8668_NUM_CHANNELS];
uint16_t adc_readings_count[ADS8668_NUM_CHANNELS];

uint16_t adc_readings_raw[ADS8668_NUM_CHANNELS];
double adc_readings[ADS8668_NUM_CHANNELS];

uint32_t adc_last_time[ADS8668_NUM_CHANNELS];
uint8_t adc_last_state[ADS8668_NUM_CHANNELS];
uint32_t adc_period[ADS8668_NUM_CHANNELS];

bool adc_enabled[ADS8668_NUM_CHANNELS];

volatile uint32_t adc_tick_count;
volatile uint32_t sensor_tick_count;

// ISRC variables
isrc_config_t isrc_configs[ADS8668_NUM_CHANNELS];
bool isrc_update;

void setup_taskSPI() {
	adc_tick_count = 0;
	sensor_tick_count = 0;
	th_tsk_SPI = osThreadNew(taskSPI, NULL, &attr_tsk_SPI);

	s_adc_dma_cplt = osSemaphoreNew(1U, 0U, &attr_s_adc_dma_cplt);
	mtx_spi1 = osMutexNew(&attr_mtx_spi1);
	mtx_adc = osMutexNew(&attr_mtx_adc);
	mtx_isrc = osMutexNew(&attr_mtx_isrc);

	// give default values to ADC
	for (uint8_t i = 0; i < ADS8668_NUM_CHANNELS; i++) {
		adc_last_time[i] = 0;
		adc_period[i] = 0;

		adc_readings_accumulator[i] = 0;
		adc_readings_count[i] = 0;

		// set to LOW
		adc_last_state[i] = 0;

		adc_change_enabled(i, true);
	}

	// give default values to ISRC
	for (uint8_t i = 0; i < ADS8668_NUM_CHANNELS; i++) {
		isrc_configs[i].update_scale = false;
		isrc_configs[i].update_value = false;
		isrc_configs[i].update_enabled = false;

		isrc_configs[i].value = 0;
	}

	isrc_update = false;

	spi_state = SPI_STATE_IDLE;
}

void adc_mutex_acquire() {
	osMutexAcquire(mtx_adc, osWaitForever);
}
void adc_mutex_release() {
	osMutexRelease(mtx_adc);
}

void adc_change_enabled(uint8_t idx, bool enabled) {
	if (idx < ADS8668_NUM_CHANNELS) {
		adc_enabled[idx] = enabled;
	}
}

void adc_change_range(uint8_t idx, uint8_t range) {
	if (idx < ADS8668_NUM_CHANNELS) {
		adc_ranges[idx].range = range;
		adc_ranges[idx].update = true;
	}
}

uint32_t adc_sample(uint8_t idx) {
	uint32_t value = 0;
	if (idx < ADS8668_NUM_CHANNELS) {
		value = adc_readings_accumulator[idx];
		if (adc_readings_count[idx] > 0) {
			value = value / adc_readings_count[idx];
			adc_readings_count[idx] = 0;
		}
		adc_readings_accumulator[idx] = 0;
	}
	return value;
}

void isrc_mutex_acquire() {
	osMutexAcquire(mtx_isrc, osWaitForever);
}

void isrc_mutex_release() {
	osMutexRelease(mtx_isrc);
}

void isrc_change_scale(uint8_t idx, uint8_t scale) {
	if (idx < ADS8668_NUM_CHANNELS) {
		isrc_configs[idx].scale = scale;
		isrc_configs[idx].update_scale = true;
		isrc_update = true;
	}
}

void isrc_change_value(uint8_t idx, uint8_t value) {
	if (idx < ADS8668_NUM_CHANNELS) {
		isrc_configs[idx].value = value;
		isrc_configs[idx].update_value = true;
		isrc_update = true;
	}
}

void isrc_change_enabled(uint8_t idx, bool enabled) {
	if (idx < ADS8668_NUM_CHANNELS) {
		isrc_configs[idx].enable = enabled;
		isrc_configs[idx].update_enabled = true;
		isrc_update = true;
	}
}

void isrc_mapping(uint8_t idx, uint8_t *maxIdx, uint8_t *channel) {
	if (idx == 0) {
		*maxIdx = MAX5548_IDX_4;
		*channel = MAX5548_CH_B;
	}
	else if (idx == 1) {
		*maxIdx = MAX5548_IDX_4;
		*channel = MAX5548_CH_A;
	}
	else if (idx == 2) {
		*maxIdx = MAX5548_IDX_1;
		*channel = MAX5548_CH_A;
	}
	else if (idx == 3) {
		*maxIdx = MAX5548_IDX_1;
		*channel = MAX5548_CH_B;
	}
	else if (idx == 4) {
		*maxIdx = MAX5548_IDX_2;
		*channel = MAX5548_CH_A;
	}
	else if (idx == 5) {
		*maxIdx = MAX5548_IDX_2;
		*channel = MAX5548_CH_B;
	}
	else if (idx == 6) {
		*maxIdx = MAX5548_IDX_3;
		*channel = MAX5548_CH_B;
	}
	else if (idx == 7) {
		*maxIdx = MAX5548_IDX_3;
		*channel = MAX5548_CH_A;
	}
}

void taskSPI(void *argument) {
	osStatus_t ret;
	uint8_t isrc_idx = 0;
	uint8_t isrc_channel = 0;

	// setup ADC
	adc_mutex_acquire();

	setup_SPI_ADS8668();
	ADS8668_Init();

	adc_change_enabled(0, true);

	adc_mutex_release();

	// setup ISRC
	isrc_mutex_acquire();

	setup_SPI_MAX5548();

	MAX5548_init(MAX5548_IDX_1);
	MAX5548_init(MAX5548_IDX_2);
	MAX5548_init(MAX5548_IDX_3);
	MAX5548_init(MAX5548_IDX_4);

	isrc_mutex_release();

	setup_SPI_ADS8668();

	// start timer
	HAL_TIM_Base_Start_IT(&htim2);

	uint8_t adc_ch = 0;
	ADS8668_cmd_ret_t adc_result;
	uint32_t flags;

	for (;;) {
//		HAL_GPIO_WritePin(PMOD_CS_GPIO_Port, PMOD_CS_Pin, GPIO_PIN_RESET);
		flags = osThreadFlagsWait(EVT_SPI_FLAG_ADC, osFlagsWaitAny,
				osWaitForever);

//		HAL_GPIO_WritePin(PMOD_CS_GPIO_Port, PMOD_CS_Pin, GPIO_PIN_SET);

		task_counter_mutex_acquire();
		task_counters.t_spi++;
		task_counter_mutex_release();

		if (flags & EVT_SPI_FLAG_ADC) {
			// woken up, so time to do ADC reading

			// acquire mutex lock on spi peripheral
			osMutexAcquire(mtx_spi1, 0U);

			spi_state = SPI_STATE_ADC_READ;

			// gonna acquire adc mutex for this entire process, as this shouldn't/can't be interrupted
			adc_mutex_acquire();

			for (uint8_t i = 0; i < ADS8668_NUM_CHANNELS; i++) {

				// determine next channel to read
				for (uint8_t i = 0; i < ADS8668_NUM_CHANNELS; i++) {
					uint8_t check_idx = (adc_ch + 1 + i) % ADS8668_NUM_CHANNELS;
					if (adc_enabled[check_idx]) {
						adc_ch = check_idx;
						break;
					}
				}

				// set pin high for timing measurement
//			HAL_GPIO_WritePin(PMOD_CS_GPIO_Port, PMOD_CS_Pin, GPIO_PIN_SET);

				adc_result = ADS8668_WriteCmd(
						ADS8668_CMD_MAN_CH_0 + (4 * adc_ch));

				// clear pin for timing measurement
//			HAL_GPIO_WritePin(PMOD_CS_GPIO_Port, PMOD_CS_Pin, GPIO_PIN_RESET);

				// use result to update values
				if (adc_result.channel_address < 8) {

					uint8_t adc_idx = adc_result.channel_address;
					adc_readings_raw[adc_idx] = adc_result.value;
					adc_readings[adc_idx] = (adc_readings_raw[adc_idx]) * 1.25;

					// add to accumulator
					adc_readings_accumulator[adc_idx] +=
							adc_readings_raw[adc_idx];
					adc_readings_count[adc_idx] += 1;

					uint8_t adc_prev_state = adc_last_state[adc_idx];
					if (adc_readings[adc_idx] > ADC_THRESH_HIGH) {
						adc_last_state[adc_idx] = 1;

						// if prev == LOW && current == HIGH -> calculate period
						if (adc_prev_state == 0) {

							// period calculated in microseconds
							adc_period[adc_idx] = (adc_tick_count
									- adc_last_time[adc_idx]) * ADC_SAMPLE_US;
							adc_last_time[adc_idx] = adc_tick_count;
						}
					}
					else if (adc_readings[adc_idx] < ADC_THRESH_LOW) {
						adc_last_state[adc_idx] = 0;
					}
				}
			}

			// check if any scale updates need to be made
			// should really only happen at beginning of program but oh well
			for (uint8_t i = 0; i < ADS8668_NUM_CHANNELS; i++) {
				if (adc_ranges[i].update) {

					// send update over SPI
					ADS8668_SetRange(i, adc_ranges[i].range);

					// mark this update as handled
					adc_ranges[i].update = false;
				}
			}

			// finished with adc, release mutex
			adc_mutex_release();

			// finished reading
			spi_state = SPI_STATE_IDLE;

			isrc_mutex_acquire();

			if (isrc_update) {
				// configure SPI for MAX5548
				setup_SPI_MAX5548();

				spi_state = SPI_STATE_ISRC;

				for (uint8_t i = 0; i < ADS8668_NUM_CHANNELS; i++) {
					isrc_mapping(i, &isrc_idx, &isrc_channel);

					if (isrc_configs[i].update_scale) {
						MAX5548_SetChannelScale(isrc_idx, isrc_channel,
								isrc_configs[i].scale);
						isrc_configs[i].update_scale = false;
					}

					if (isrc_configs[i].update_value) {
						MAX5548_SetChannelValue(isrc_idx, isrc_channel,
								isrc_configs[i].value);
						isrc_configs[i].update_value = false;
					}

					if (isrc_configs[i].update_enabled) {
						MAX5548_EnableChannel(isrc_idx, isrc_channel,
								isrc_configs[i].enable);
						isrc_configs[i].update_enabled = false;
					}
				}

				isrc_update = false;

				// configure SPI back for ADC
				setup_SPI_ADS8668();

				// finished
				spi_state = SPI_STATE_IDLE;
			}

			isrc_mutex_release();

			// release mutex
			osMutexRelease(mtx_spi1);
		}
	}

	// somehow exited, terminate task
	osThreadTerminate(NULL);
}

void tim2_cb() {
	// increment tick count for PWM sampling
	adc_tick_count++;

	// signal SPI thread to perform ADC reading
	osThreadFlagsSet(th_tsk_SPI, EVT_SPI_FLAG_ADC);

	sensor_tick_count++;

	if (sensor_tick_count >= SENSOR_UPDATE_TICK) {
		sensor_tick_count = 0;
		osThreadFlagsSet(th_tsk_main, MAIN_EVT_FLAG_SENSOR);
	}
}

void spi1_TxRxCplt_cb() {
	if (spi_state == SPI_STATE_ADC_READ) {
		// this dma callback is result of adc reading

		// post semaphore to notify task
		osSemaphoreRelease(s_adc_dma_cplt);
	}
}

void setup_SPI_ADS8668() {
	// configure for 15MHz clock (60 MHz / 4)

	// disable SPI
	LL_SPI_Disable(SPI1);

	LL_SPI_SetClockPolarity(SPI1, LL_SPI_POLARITY_LOW);
	LL_SPI_SetClockPhase(SPI1, LL_SPI_PHASE_2EDGE);
	LL_SPI_SetBaudRatePrescaler(SPI1, LL_SPI_BAUDRATEPRESCALER_DIV4);

	// enable SPI
	LL_SPI_Enable(SPI1);
}

void setup_SPI_MAX5548() {
	// configure for 7.5MHz clock (60 MHz / 8)

	// disable SPI
	LL_SPI_Disable(SPI1);

	LL_SPI_SetClockPolarity(SPI1, LL_SPI_POLARITY_LOW);
	LL_SPI_SetClockPhase(SPI1, LL_SPI_PHASE_1EDGE);
	LL_SPI_SetBaudRatePrescaler(SPI1, LL_SPI_BAUDRATEPRESCALER_DIV8);

	// enable SPI
	LL_SPI_Enable(SPI1);
}
