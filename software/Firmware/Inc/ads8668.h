/*
 * ads8668.h
 *
 *  Created on: Feb 9, 2022
 *      Author: Calvin
 */

#ifndef INC_ADS8668_H_
#define INC_ADS8668_H_

#include <stm32f2xx_hal.h>
#include <string.h>

#include <window_filtering.h>

#define ADS8668_NUM_CHANNELS	8

#define ADS8668_RANGE_10V24_DIFF	0b0000
#define ADS8668_RANGE_5V12_DIFF		0b0001
#define ADS8668_RANGE_2V56_DIFF		0b0010
#define ADS8668_RANGE_1V28_DIFF		0b0011
#define ADS8668_RANGE_0V64_DIFF		0b1011
#define ADS8668_RANGE_10V24			0b0101
#define ADS8668_RANGE_5V12			0b0110
#define ADS8668_RANGE_2V56			0b0111
#define ADS8668_RANGE_1V28			0b1111

#define ADS8668_CMD_NO_OP	0x00

#define ADS8668_MAN_CH_0 	0xC0
#define ADS8668_MAN_CH_1 	0xC4
#define ADS8668_MAN_CH_2 	0xC8
#define ADS8668_MAN_CH_3 	0xCC
#define ADS8668_MAN_CH_4 	0xD0
#define ADS8668_MAN_CH_5 	0xD4
#define ADS8668_MAN_CH_6 	0xD8
#define ADS8668_MAN_CH_7 	0xDC

#define ADC_FILTER_SIZE 32

void ADS8668_CS_ON();
void ADS8668_CS_OFF();

void ADS8668_Reset();
void ADS8668_Init();

void ADS8668_FilterEnable(uint8_t channel);
void ADS8668_FilterDisable(uint8_t channel);

uint8_t ADS8668_WriteReg(uint8_t reg, uint8_t data);
uint8_t ADS8668_ReadReg(uint8_t reg);

uint16_t ADS8668_WriteCmd(uint8_t cmdReg, uint8_t value);
void ADS8668_WriteCmd_DMA(uint8_t cmdReg, uint8_t value);

void ADS8668_ReadAll_DMA_Start();

void ADS8668_Background_Disable();
void ADS8668_Background_Enable();

void ADS8668_UpdateValues();
void ADS8668_ReadAll();

void HAL_SPI_TxRxCpltCallback_ADS8668(SPI_HandleTypeDef * hspi);

uint8_t ADS8668_WriteRegister(uint8_t reg, uint8_t value);
uint16_t ADS8668_ReadChannel(uint8_t address);
void ADS8668_SetRange(uint8_t channel, uint8_t range);

float ADS8668_GetScaledFiltered(uint8_t channel);

extern volatile uint16_t adc_readings_raw[ADS8668_NUM_CHANNELS];
extern volatile uint32_t adc_period[ADS8668_NUM_CHANNELS];
extern volatile uint32_t adc_last_time[ADS8668_NUM_CHANNELS];

extern window_filter_t adc_period_filtered[ADS8668_NUM_CHANNELS];

extern bool adc_complete;
extern bool adc_filter_enable[ADS8668_NUM_CHANNELS];
extern window_filter_t adc_filtered[ADS8668_NUM_CHANNELS];
extern float adc_readings[ADS8668_NUM_CHANNELS];

void ads8668_tim_cb();

#endif /* INC_ADS8668_H_ */
