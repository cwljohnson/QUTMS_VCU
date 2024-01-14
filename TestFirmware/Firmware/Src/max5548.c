/*
 * max5548.c
 *
 *  Created on: 23 Feb. 2022
 *      Author: Calvin
 */

#include "max5548.h"
#include "main.h"
#include "spi.h"

void MAX5548_CS_ON(uint8_t idx) {
	if (idx == 0) {
		HAL_GPIO_WritePin(ISRC_1_CS_GPIO_Port, ISRC_1_CS_Pin, GPIO_PIN_RESET);
	}
	else if (idx == 1) {
		HAL_GPIO_WritePin(ISRC_2_CS_GPIO_Port, ISRC_2_CS_Pin, GPIO_PIN_RESET);
	}
	else if (idx == 2) {
		HAL_GPIO_WritePin(ISRC_3_CS_GPIO_Port, ISRC_3_CS_Pin, GPIO_PIN_RESET);
	}
	else if (idx == 3) {
		HAL_GPIO_WritePin(ISRC_4_CS_GPIO_Port, ISRC_4_CS_Pin, GPIO_PIN_RESET);
	}

	//HAL_GPIO_WritePin(PMOD_CS_GPIO_Port, PMOD_CS_Pin, GPIO_PIN_RESET);
}

void MAX5548_CS_OFF(uint8_t idx) {
	if (idx == 0) {
		HAL_GPIO_WritePin(ISRC_1_CS_GPIO_Port, ISRC_1_CS_Pin, GPIO_PIN_SET);
	}
	else if (idx == 1) {
		HAL_GPIO_WritePin(ISRC_2_CS_GPIO_Port, ISRC_2_CS_Pin, GPIO_PIN_SET);
	}
	else if (idx == 2) {
		HAL_GPIO_WritePin(ISRC_3_CS_GPIO_Port, ISRC_3_CS_Pin, GPIO_PIN_SET);
	}
	else if (idx == 3) {
		HAL_GPIO_WritePin(ISRC_4_CS_GPIO_Port, ISRC_4_CS_Pin, GPIO_PIN_SET);
	}

	//HAL_GPIO_WritePin(PMOD_CS_GPIO_Port, PMOD_CS_Pin, GPIO_PIN_SET);
}

void MAX5548_SendCMD(uint8_t idx, uint8_t cmd) {
	MAX5548_CS_ON(idx);
	HAL_Delay(1);

	uint8_t tx[2];
	tx[0] = (cmd & 0x3F) << 2;
	tx[1] = 0;

	HAL_StatusTypeDef result = HAL_SPI_Transmit(&hspi1, tx, 2, 2000);

	HAL_Delay(1);
	MAX5548_CS_OFF(idx);
	HAL_Delay(1);
}

void MAX5548_SendDATA(uint8_t idx, uint8_t cmd, uint8_t data) {
	MAX5548_CS_ON(idx);
	HAL_Delay(1);

	uint8_t tx[2];
	tx[0] = ((cmd & 0x3F) << 2) | ((data & 0xC0) >> 6);
	tx[1] = (data & 0x3F) << 2;

	HAL_StatusTypeDef result = HAL_SPI_Transmit(&hspi1, tx, 2, 2000);
	HAL_Delay(1);
	MAX5548_CS_OFF(idx);
	HAL_Delay(1);
}

void MAX5548_EnableChannel(uint8_t idx, uint8_t channel, bool value) {
	MAX5548_SPI1_Init();

	if (channel == MAX5548_CH_A) {
		if (value) {
			MAX5548_SendCMD(idx, MAX5548_CMD_POWER_A);
		}
		else {
			MAX5548_SendCMD(idx, MAX5548_CMD_SHUTDOWN_A);
		}
	}
	else if (channel == MAX5548_CH_B) {
		if (value) {
			MAX5548_SendCMD(idx, MAX5548_CMD_POWER_B);
		}
		else {
			MAX5548_SendCMD(idx, MAX5548_CMD_SHUTDOWN_B);
		}
	}
}

void MAX5548_SetChannelScale(uint8_t idx, uint8_t channel, uint8_t scale) {
	MAX5548_SPI1_Init();

	if (channel == MAX5548_CH_A) {
		MAX5548_SendCMD(idx, MAX5548_CMD_SCALE_A | scale);
	}
	else if (channel == MAX5548_CH_B) {
		MAX5548_SendCMD(idx, MAX5548_CMD_SCALE_B | scale);
	}
}

void MAX5548_SetChannelValue(uint8_t idx, uint8_t channel, uint8_t value) {
	MAX5548_SPI1_Init();

	if (channel == MAX5548_CH_A) {
		MAX5548_SendDATA(idx, MAX5548_CMD_LOAD_DAC_A, value);
	}
	else if (channel == MAX5548_CH_B) {
		MAX5548_SendDATA(idx, MAX5548_CMD_LOAD_DAC_B, value);
	}
}

void MAX5548_init(uint8_t idx) {
	MAX5548_SPI1_Init();

	// use internal reference
	MAX5548_SendCMD(idx, MAX5548_CMD_REF_INTERNAL);

	// leave both channels off initially
	MAX5548_SendCMD(idx, MAX5548_CMD_SHUTDOWN_BOTH);
}
