/*
 * ads8668.c
 *
 *  Created on: Feb 6, 2023
 *      Author: Calvin
 */

#include "ads8668.h"
#include "spi.h"
#include "cmsis_os.h"

uint8_t tx[5];
uint8_t rx[5];

void ADS8668_CS_ON() {
	HAL_GPIO_WritePin(ADC_CS_GPIO_Port, ADC_CS_Pin, GPIO_PIN_RESET);
}

void ADS8668_CS_OFF() {
	HAL_GPIO_WritePin(ADC_CS_GPIO_Port, ADC_CS_Pin, GPIO_PIN_SET);
}

void ADS8668_Reset() {
	// enter PWR_DN by setting RST to low for > 400ns
	HAL_GPIO_WritePin(ADC_RST_GPIO_Port, ADC_RST_Pin, GPIO_PIN_RESET);

	// 1 ms > 400ns
	osDelay(1);

	// wake device back up by setting RST to high
	HAL_GPIO_WritePin(ADC_RST_GPIO_Port, ADC_RST_Pin, GPIO_PIN_SET);

	// execute valid 16bit write command to exit PWR_DN
	// MAN_CH_0 is a valid command
	ADS8668_WriteCmd(ADS8668_CMD_MAN_CH_0);

	// 15ms required to power circuitry -> 16ms
	osDelay(16);

}

void ADS8668_Init() {
	// reset everything
	ADS8668_Reset();

	// configure all channels @ 5.12V
	for (uint8_t i = 0; i < ADS8668_NUM_CHANNELS; i++) {
		//ADS8668_WriteReg(0x05 + i, ADS8668_RANGE_5V12);
		ADS8668_SetRange(i, ADS8668_RANGE_5V12);
	}

	// update feature select to change output type
	// 0b001 -> transmit channel address
	ADS8668_WriteReg(ADS8668_REG_FEAT_SEL, 0x1);
}

ADS8668_cmd_ret_t ADS8668_WriteCmd(uint8_t cmdReg) {
	tx[0] = cmdReg;
	tx[1] = 0;
	tx[2] = 0;
	tx[3] = 0;

	ADS8668_CS_ON();
	SPI1_TxRx(tx, rx, 5);
	ADS8668_CS_OFF();

	ADS8668_cmd_ret_t result;
	result.value = ((uint16_t) rx[2] << 4) | ((rx[3] >> 4) & 0xF);
	result.channel_address = (rx[4] >> 4) & 0xF;

	return result;
}

void ADS8668_WriteCmd_DMA(uint8_t cmdReg) {
	tx[0] = cmdReg;
	tx[1] = 0;
	tx[2] = 0;
	tx[3] = 0;

	ADS8668_CS_ON();

	SPI1_TxRx_DMA_Start(tx, rx, 5);
}

ADS8668_cmd_ret_t ADS8668_DMA_result_WriteCMD() {
	ADS8668_CS_OFF();

	ADS8668_cmd_ret_t result;
	result.value = ((uint16_t) rx[2] << 4) | ((rx[3] >> 4) & 0xF);
	result.channel_address = (rx[4] >> 4) & 0xF;

	return result;
}

void ADS8668_SetRange(uint8_t idx, uint8_t range) {
	if (idx < ADS8668_NUM_CHANNELS) {
		ADS8668_WriteReg(0x05 + idx, range);
	}
}

uint8_t ADS8668_WriteReg(uint8_t reg, uint8_t data) {
	tx[0] = (reg << 1) | 0x1;
	tx[1] = data;
	tx[2] = 0;

	ADS8668_CS_ON();
	SPI1_TxRx(tx, rx, 3);
	ADS8668_CS_OFF();

	return rx[2];
}

uint8_t ADS8668_ReadReg(uint8_t reg) {
	uint8_t tx[3];
	uint8_t rx[3];

	tx[0] = (reg << 1) | 0x0;
	tx[1] = 0;
	tx[2] = 0;

	ADS8668_CS_ON();
	SPI1_TxRx(tx, rx, 3);
	ADS8668_CS_OFF();

	return rx[2];
}
