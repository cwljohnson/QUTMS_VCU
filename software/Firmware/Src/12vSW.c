/*
 * 12vSW.c
 *
 *  Created on: Oct 10, 2021
 *      Author: Calvin
 */


#include "12vSW.h"
#include "main.h"

void SW_setState(uint8_t idx, bool enable) {
	if (idx == 0) {
		HAL_GPIO_WritePin(PROFET_IN0_GPIO_Port, PROFET_IN0_Pin, enable ? GPIO_PIN_SET : GPIO_PIN_RESET);
	} else if (idx == 1) {
		HAL_GPIO_WritePin(PROFET_IN1_GPIO_Port, PROFET_IN1_Pin, enable ? GPIO_PIN_SET : GPIO_PIN_RESET);
	}
}
