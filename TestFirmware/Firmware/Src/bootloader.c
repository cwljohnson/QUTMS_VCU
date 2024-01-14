/*
 * bootloader.c
 *
 *  Created on: Jun 7, 2022
 *      Author: Calvin
 */

#include "bootloader.h"
#include "main.h"

void triggerBootloader() {
	// set pin high
	HAL_GPIO_WritePin(BOOT_CTRL_GPIO_Port, BOOT_CTRL_Pin, GPIO_PIN_SET);

	// wait for cap to charge
	HAL_Delay(100);

	// force system reset to open bootloader
	NVIC_SystemReset();
}
