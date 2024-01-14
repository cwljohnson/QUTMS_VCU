/*
 * ads8668.c
 *
 *  Created on: Feb 9, 2022
 *      Author: Calvin
 */

#include "ads8668.h"
#include "main.h"
#include "spi.h"
#include "tim.h"

#include <stdbool.h>

uint8_t tx[4];
uint8_t rx[4];

bool adc_filter_enable[ADS8668_NUM_CHANNELS];

volatile uint16_t adc_readings_raw[ADS8668_NUM_CHANNELS];
volatile uint32_t adc_period[ADS8668_NUM_CHANNELS];
volatile uint32_t adc_last_time[ADS8668_NUM_CHANNELS];

window_filter_t adc_period_filtered[ADS8668_NUM_CHANNELS];

window_filter_t adc_filtered[ADS8668_NUM_CHANNELS];
float adc_scalers[ADS8668_NUM_CHANNELS];
int16_t adc_offset[ADS8668_NUM_CHANNELS];
float adc_readings[ADS8668_NUM_CHANNELS];

bool adc_update_required;

volatile uint8_t adc_idx;
volatile bool readingFinished = false;

volatile uint32_t timer_count;

int dma_idx;

volatile bool spi_adc_running;
volatile bool spi_adc_req_stop;

void ADS8668_Reset() {
	ADS8668_CS_OFF();

	HAL_GPIO_WritePin(ADC_RST_GPIO_Port, ADC_RST_Pin, GPIO_PIN_RESET);
	HAL_Delay(5);
	HAL_GPIO_WritePin(ADC_RST_GPIO_Port, ADC_RST_Pin, GPIO_PIN_SET);
}

void ADS8668_Init() {
	ADS8668_SPI1_Init();

	timer_count = 0;
	ADS8668_Reset();

	// set internal reference
	HAL_GPIO_WritePin(ADC_REFSEL_GPIO_Port, ADC_REFSEL_Pin, GPIO_PIN_RESET);
	HAL_Delay(2);

	// send valid command to exit PWR_DN
	ADS8668_WriteCmd(ADS8668_CMD_NO_OP, 0x00);

	// wait 15ms for power up
	HAL_Delay(15);

	// set voltage range
	//ADS8668_WriteReg(0x05 + 3, ADS8668_RANGE_5V12);

	for (uint8_t i = 0; i < ADS8668_NUM_CHANNELS; i++) {
		ADS8668_SetRange(i, ADS8668_RANGE_5V12);

		ADS8668_FilterDisable(i);

		adc_last_time[i] = HAL_GetTick();
	}

	// send valid command to exit PWR_DN
	ADS8668_WriteCmd(ADS8668_CMD_NO_OP, 0x00);

	readingFinished = true;
	adc_idx = 0;

	spi_adc_running = false;

	// start timer
	//HAL_TIM_Base_Start_IT(&htim2);
}

void ADS8668_CS_ON() {
	//HAL_GPIO_WritePin(PMOD_CS_GPIO_Port, PMOD_CS_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(ADC_CS_GPIO_Port, ADC_CS_Pin, GPIO_PIN_RESET);
}

void ADS8668_CS_OFF() {
	//HAL_GPIO_WritePin(PMOD_CS_GPIO_Port, PMOD_CS_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(ADC_CS_GPIO_Port, ADC_CS_Pin, GPIO_PIN_SET);
}

void ADS8668_FilterEnable(uint8_t channel) {
	adc_filter_enable[channel] = true;
}

void ADS8668_FilterDisable(uint8_t channel) {
	adc_filter_enable[channel] = false;
}

uint16_t ADS8668_WriteCmd(uint8_t cmdReg, uint8_t value) {
	tx[0] = cmdReg;
	tx[1] = value;
	tx[2] = 0;
	tx[3] = 0;

	ADS8668_CS_ON();
	HAL_SPI_TransmitReceive(&hspi1, tx, rx, 4, 1000);
	ADS8668_CS_OFF();

	uint16_t result = ((rx[2] << 8) | rx[3]) >> 4;
	return result;
}

void ADS8668_WriteCmd_DMA(uint8_t cmdReg, uint8_t value) {
	readingFinished = false;
	//HAL_GPIO_WritePin(PMOD_INT_GPIO_Port, PMOD_INT_Pin, GPIO_PIN_SET);
	tx[0] = cmdReg;
	tx[1] = value;
	tx[2] = 0;
	tx[3] = 0;

	ADS8668_CS_ON();
	HAL_SPI_TransmitReceive_DMA(&hspi1, tx, rx, 4);
}

void ADS8668_Background_Disable() {
	spi_adc_req_stop = true;
	HAL_TIM_Base_Stop_IT(&htim2);

	if (!spi_adc_running) {
		return;
	}
	else {
		// wait 1ms then reading should be finished
		HAL_Delay(1);
	}

	// reading finished, all disabled

	return;
}

void ADS8668_Background_Enable() {

	// initialize ADS8668 again
	ADS8668_SPI1_Init();

	// reset timer count for period stuff
	timer_count = 0;

	spi_adc_req_stop = false;
	HAL_TIM_Base_Start_IT(&htim2);
}

void ADS8668_ReadAll_DMA_Start() {
	spi_adc_running = true;

	uint8_t cmdReg = ADS8668_MAN_CH_0 + (4 * adc_idx);
	ADS8668_WriteCmd_DMA(cmdReg, 0x00);
}

void ADS8668_UpdateValues() {
	//if (adc_update_required) {
	for (int i = 0; i < ADS8668_NUM_CHANNELS; i++) {
		adc_readings[i] = (adc_readings_raw[i] + adc_offset[i]) * adc_scalers[i];

		if (adc_filter_enable[i]) {
			window_filter_update(&adc_filtered[i], adc_readings_raw[i]);
		}
	}

}

void HAL_SPI_TxRxCpltCallback_ADS8668(SPI_HandleTypeDef *hspi) {
	ADS8668_CS_OFF();
	readingFinished = true;
	spi_adc_running = false;

	//HAL_GPIO_TogglePin(PMOD_INT_GPIO_Port, PMOD_INT_Pin);

	//HAL_GPIO_WritePin(PMOD_INT_GPIO_Port, PMOD_INT_Pin, GPIO_PIN_RESET);

	uint8_t idx = (adc_idx - 1 + 8) % 8;

	uint16_t adcPrev = adc_readings_raw[idx];
	adc_readings_raw[idx] = ((rx[2] << 8) | rx[3]) >> 4;
	adc_idx = (adc_idx + 1) % 8;
/*
	adc_readings[idx] = (adc_readings_raw[idx] + adc_offset[idx]) * adc_scalers[idx];

	if (adc_filter_enable[idx]) {
		window_filter_update(&adc_filtered[idx], adc_readings_raw[idx]);
	}
*/
	bool prevHigh = adcPrev > 2000;
	bool currentHigh = adc_readings_raw[idx] > 2000;

	if ((currentHigh != prevHigh) && (prevHigh)) {
		// state change and falling edge -> in microseconds
		adc_period[idx] = (timer_count - adc_last_time[idx]) * 26;
		adc_last_time[idx] = timer_count;
		//window_filter_update(&adc_period_filtered[idx], adc_period[idx]);
	}

	if (adc_readings_raw[3] > 2000) {
		HAL_GPIO_WritePin(PMOD_INT_GPIO_Port, PMOD_INT_Pin, GPIO_PIN_SET);
	}
	else {
		HAL_GPIO_WritePin(PMOD_INT_GPIO_Port, PMOD_INT_Pin, GPIO_PIN_RESET);
	}

}

void ADS8668_ReadAll() {
	ADS8668_SPI1_Init();

	for (uint8_t i = 0; i <= 8; i++) {
		uint8_t cmd = 0;

		if (i < 8) {
			cmd = ADS8668_MAN_CH_0 + (4 * i);
		}
		else {
			cmd = ADS8668_CMD_NO_OP;
		}

		uint16_t output = ADS8668_WriteCmd(cmd, 0x00);

		if (i > 0) {
			adc_readings_raw[i - 1] = output;
		}
	}

	ADS8668_UpdateValues();
}

uint8_t ADS8668_WriteReg(uint8_t reg, uint8_t data) {
	tx[0] = (reg << 1) | 1;
	tx[1] = data;
	tx[2] = 0;

	ADS8668_CS_ON();
	HAL_SPI_TransmitReceive(&hspi1, tx, rx, 3, 1000);
	ADS8668_CS_OFF();

	uint8_t result = rx[2];

	return result;
}

uint8_t ADS8668_ReadReg(uint8_t reg) {
	tx[0] = (reg << 1) | 0;
	tx[1] = 0;
	tx[2] = 0;

	ADS8668_CS_ON();
	HAL_SPI_TransmitReceive(&hspi1, tx, rx, 3, 2000);
	ADS8668_CS_OFF();

	uint8_t result = rx[2];
	return result;
}

void ADS8668_SetRange(uint8_t channel, uint8_t range) {
	ADS8668_SPI1_Init();

	uint16_t addr = 0x05 + channel;

	if (range == ADS8668_RANGE_10V24_DIFF) {
		adc_scalers[channel] = 2.5f;
		adc_offset[channel] = -2047;
	}
	else if (range == ADS8668_RANGE_5V12_DIFF) {
		adc_scalers[channel] = 1.25f;
		adc_offset[channel] = -2047;
	}
	else if (range == ADS8668_RANGE_2V56_DIFF) {
		adc_scalers[channel] = 0.625f;
		adc_offset[channel] = -2047;
	}
	else if (range == ADS8668_RANGE_1V28_DIFF) {
		adc_scalers[channel] = 0.3125f;
		adc_offset[channel] = -2047;
	}
	else if (range == ADS8668_RANGE_0V64_DIFF) {
		adc_scalers[channel] = 0.15625f;
		adc_offset[channel] = -2047;
	}
	else if (range == ADS8668_RANGE_10V24) {
		adc_scalers[channel] = 2.5f;
		adc_offset[channel] = 0;
	}
	else if (range == ADS8668_RANGE_5V12) {
		adc_scalers[channel] = 1.25f;
		adc_offset[channel] = 0;
	}
	else if (range == ADS8668_RANGE_2V56) {
		adc_scalers[channel] = 0.625f;
		adc_offset[channel] = 0;
	}
	else if (range == ADS8668_RANGE_1V28) {
		adc_scalers[channel] = 0.3125f;
		adc_offset[channel] = 0;
	}

	uint8_t result = ADS8668_WriteReg(addr, range);
}

float ADS8668_GetScaledFiltered(uint8_t channel) {
	return (adc_filtered[channel].current_filtered + adc_offset[channel]) * adc_scalers[channel];
}

void ads8668_tim_cb() {
	// only run if not requested to stop
	if (!spi_adc_req_stop) {
		timer_count++;

		//HAL_GPIO_TogglePin(PMOD_INT_GPIO_Port, PMOD_INT_Pin);
		if (readingFinished) {
			ADS8668_ReadAll_DMA_Start();
			//ADS8668_WriteCmd_IT(ADS8668_CMD_MAN_CH_3, 0x00);
		}
	}
}

