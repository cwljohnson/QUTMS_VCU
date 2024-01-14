/*
 * sensor_rtd.c
 *
 *  Created on: Sep 7, 2023
 *      Author: Calvin
 */

#include "sensor_rtd.h"

#include "p_12vSW.h"
#include "task_spi.h"

#include <CAN_VCU.h>

static bool rtd_btn_state;

void sensor_rtd_update_outputs(state_machine_t *state_machine) {
	if (state_machine->state_current == VCU_STATE_IDLE) {
		SW_setState(SW_IDX_P_RTD_BTN, true);
	}
	else if (state_machine->state_current == VCU_STATE_RTD_RDY) {
		// flash LED with 500ms period
		if ((state_machine->current_ticks % 500) < 250) {
			SW_setState(SW_IDX_P_RTD_BTN, true);

		}
		else {
			SW_setState(SW_IDX_P_RTD_BTN, false);

		}
	}
	else if (state_machine->state_current == VCU_STATE_RTD_BTN) {
		SW_setState(SW_IDX_P_RTD_BTN, true);
	}
	else if (state_machine->state_current == VCU_STATE_DRIVING) {
		SW_setState(SW_IDX_P_RTD_BTN, true);
	}
	else {
		SW_setState(SW_IDX_P_RTD_BTN, false);
	}

	if ((state_machine->state_current == VCU_STATE_DRIVING)
			|| (state_machine->state_current == VCU_STATE_DVL_RTD)) {
		// siren only on for first 2s
		if (state_machine->current_ticks < 2000U) {
			SW_setState(SW_IDX_P_RTD_SIREN, true);
		}
		else {
			SW_setState(SW_IDX_P_RTD_SIREN, false);
		}
	}
	else if (state_machine->state_current == VCU_STATE_DVL_EMERGENCY) {
		// flash LED with 500ms period
		if (state_machine->current_ticks < 9000U) {
			if ((state_machine->current_ticks % 250) < 125) {
				SW_setState(SW_IDX_P_RTD_SIREN, true);

			}
			else {
				SW_setState(SW_IDX_P_RTD_SIREN, false);

			}
		}
		else {
			SW_setState(SW_IDX_P_RTD_SIREN, false);
		}

	}
	else {
		SW_setState(SW_IDX_P_RTD_SIREN, false);
	}
}

void sensor_setup_rtd_btn(void) {
	// these channels are on LHS of VCU, so enable current sources for pullup
	isrc_mutex_acquire();

	// configure for ~ 1mA
	isrc_change_scale(SENSOR_IDX_P_RTD_BTN, MAX5548_SCALE_0);

	isrc_change_value(SENSOR_IDX_P_RTD_BTN, 10);

	isrc_change_enabled(SENSOR_IDX_P_RTD_BTN, true);

	isrc_mutex_release();

	rtd_btn_state = false;
}

void sensor_update_rtd_btn(sensor_data_t *sensor_data) {
	uint16_t adc_rtd_btn = sensor_data->adc_filtered[SENSOR_IDX_P_RTD_BTN]
			* ADC_SCALE_5V12;
	rtd_btn_state = (adc_rtd_btn < 1000);
}

bool rtd_get_btn(void) {
	return rtd_btn_state;
}
