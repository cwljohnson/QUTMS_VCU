/*
 * p_12vSW.c
 *
 *  Created on: Sep 7, 2023
 *      Author: Calvin
 */

#include "p_12vSW.h"
#include "main.h"

static volatile bool sw0_enabled = false;
static volatile bool sw1_enabled = false;

static volatile uint8_t sw0_dc = 0;
static volatile uint8_t sw1_dc = 0;

static volatile uint8_t sw0_dc_target = 0;
static volatile uint8_t sw1_dc_target = 0;


void SW_setState(uint8_t idx, bool enable) {
	if (idx == 0) {
		HAL_GPIO_WritePin(PROFET_IN0_GPIO_Port, PROFET_IN0_Pin, enable ? GPIO_PIN_SET : GPIO_PIN_RESET);
		sw0_enabled = false;
	} else if (idx == 1) {
		HAL_GPIO_WritePin(PROFET_IN1_GPIO_Port, PROFET_IN1_Pin, enable ? GPIO_PIN_SET : GPIO_PIN_RESET);
		sw1_enabled = false;
	}
}

void SW_setPWM(uint8_t idx, uint8_t duty_cycle) {
	if (idx == 0) {
		sw0_enabled = true;
		sw0_dc_target = duty_cycle;
	} else if (idx == 1) {
		sw1_enabled = true;
		sw1_dc_target = duty_cycle;
	}


}

void pwm12V_timer_cb(void) {
	// this is called at 10 Hz
	static uint16_t pwm_counter = 0;
	pwm_counter = (pwm_counter + 1) % 100;

	if (sw0_enabled) {
		if (pwm_counter == 0) {
			// been 1s, increment DC if needed
			if (sw0_dc_target > sw0_dc) {
				sw0_dc++;
			}
		}

		bool enable = sw0_dc > (pwm_counter % 10);
		HAL_GPIO_WritePin(PROFET_IN0_GPIO_Port, PROFET_IN0_Pin, enable ? GPIO_PIN_SET : GPIO_PIN_RESET);
	}

	if (sw1_enabled) {
		if (pwm_counter == 0) {
			// been 1s, increment DC if needed
			if (sw1_dc_target > sw1_dc) {
				sw1_dc++;
			}
		}

		bool enable = sw1_dc > (pwm_counter % 10);
		HAL_GPIO_WritePin(PROFET_IN1_GPIO_Port, PROFET_IN1_Pin, enable ? GPIO_PIN_SET : GPIO_PIN_RESET);
	}
}
