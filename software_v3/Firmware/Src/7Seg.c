/*
 * 7Seg.c
 *
 *  Created on: Nov 4, 2023
 *      Author: Calvin
 */

#include "7Seg.h"
#include "main.h"

uint8_t seg_lookup[16] = { 0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F, 0x77, 0x7C, 0x39, 0x5E, 0x79,
		0x71 };

void set_7seg(uint8_t input, bool dot) {

	if (input <= 0xF) {
		uint8_t value = seg_lookup[input];

		HAL_GPIO_WritePin(SA_GPIO_Port, SA_Pin, ((value >> 0) & 1) == 1 ? GPIO_PIN_RESET : GPIO_PIN_SET);
		HAL_GPIO_WritePin(SB_GPIO_Port, SB_Pin, ((value >> 1) & 1) == 1 ? GPIO_PIN_RESET : GPIO_PIN_SET);
		HAL_GPIO_WritePin(SC_GPIO_Port, SC_Pin, ((value >> 2) & 1) == 1 ? GPIO_PIN_RESET : GPIO_PIN_SET);
		HAL_GPIO_WritePin(SD_GPIO_Port, SD_Pin, ((value >> 3) & 1) == 1 ? GPIO_PIN_RESET : GPIO_PIN_SET);
		HAL_GPIO_WritePin(SE_GPIO_Port, SE_Pin, ((value >> 4) & 1) == 1 ? GPIO_PIN_RESET : GPIO_PIN_SET);
		HAL_GPIO_WritePin(SF_GPIO_Port, SF_Pin, ((value >> 5) & 1) == 1 ? GPIO_PIN_RESET : GPIO_PIN_SET);
		HAL_GPIO_WritePin(SG_GPIO_Port, SG_Pin, ((value >> 6) & 1) == 1 ? GPIO_PIN_RESET : GPIO_PIN_SET);
	}

	HAL_GPIO_WritePin(SDP_GPIO_Port, SDP_Pin, dot ? GPIO_PIN_RESET : GPIO_PIN_SET);
}
