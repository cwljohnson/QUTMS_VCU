/*
 * task_spi.h
 *
 *  Created on: Feb 6, 2023
 *      Author: Calvin
 */

#ifndef INC_TASK_SPI_H_
#define INC_TASK_SPI_H_

#include "max5548.h"
#include "cmsis_os.h"

#include <stdint.h>
#include <stdbool.h>

extern osSemaphoreId_t s_tim_adc;

typedef enum {
	EVT_SPI_FLAG_ADC = 0x1
} EVT_SPI;

typedef enum {
	SPI_STATE_IDLE,
	SPI_STATE_ADC_READ,
	SPI_STATE_ISRC
} SPI1_STATE;

typedef struct adc_range {
	bool update;
	uint8_t range;
} adc_range_t;

typedef struct isrc_config {
	bool update_scale;
	bool update_value;
	bool update_enabled;
	uint8_t scale;
	uint8_t value;
	bool enable;
} isrc_config_t;

#define ADC_THRESH_HIGH 4000
#define ADC_THRESH_LOW 1000
#define ADC_SAMPLE_US 100

#define ADC_CUTOFF_PULLDOWN (100)

#define ADC_SCALE_5V12 (1.25)

void setup_taskSPI();

void tim2_cb();
void spi1_TxRxCplt_cb();

void setup_SPI_ADS8668();
void setup_SPI_MAX5548();

void adc_mutex_acquire();
void adc_mutex_release();

void adc_change_enabled(uint8_t idx, bool enabled);
void adc_change_range(uint8_t idx, uint8_t range);
uint32_t adc_sample(uint8_t idx);


void isrc_mutex_acquire();
void isrc_mutex_release();

void isrc_mapping(uint8_t idx, uint8_t *maxIdx, uint8_t *channel);

void isrc_change_scale(uint8_t idx, uint8_t scale);
void isrc_change_value(uint8_t idx, uint8_t value);
void isrc_change_enabled(uint8_t idx, bool enabled);

#endif /* INC_TASK_SPI_H_ */
