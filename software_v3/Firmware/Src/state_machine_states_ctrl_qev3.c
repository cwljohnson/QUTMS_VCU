/*
 * state_machine_states_ctrl_qev3.c
 *
 *  Created on: Dec 7, 2023
 *      Author: Calvin
 */

#include "state_machine.h"

#include <CAN_MCISO.h>

#include "heartbeat.h"
#include "sensor_rtd.h"
#include "sensor_pedalBox.h"
#include "inv_vesc.h"

#if QEV3 == 1

VCU_STATE state_inverter_check(state_machine_t *state_machine) {
	VCU_STATE new_state = VCU_STATE_INVERTER_CHECK;

	bool inverter_good = true;

	for (uint8_t i = 0; i < MCISO_COUNT; i++) {
		// check MCISO board is good
		inverter_good = inverter_good && heartbeat_states.hb_MCISO[i].hb_alive;

		// check connected inverters are good
		inverter_good = inverter_good
				&& heartbeat_boards.MCISO_hbState[i].errorFlags.HB_INV0 == 0;
		inverter_good = inverter_good
				&& heartbeat_boards.MCISO_hbState[i].errorFlags.HB_INV1 == 0;
	}

	if (inverter_good) {
		new_state = VCU_STATE_RTD_RDY;
		return new_state;
	}
	return new_state;
}

VCU_STATE state_rtd_rdy(state_machine_t *state_machine) {
	VCU_STATE new_state = VCU_STATE_RTD_RDY;

	pedal_values_t pedal_values;
	sensor_pedalBox_getData(&pedal_values);

	// brake has to be pressed
	if (pedal_values.brake_pressed) {
		new_state = VCU_STATE_RTD_BTN;
		return new_state;
	}

	return new_state;
}

VCU_STATE state_rtd_btn(state_machine_t *state_machine) {
	VCU_STATE new_state = VCU_STATE_RTD_BTN;

	static uint32_t press_start = 0;

	pedal_values_t pedal_values;
	sensor_pedalBox_getData(&pedal_values);

	// if brake isn't pressed go back lol
	if (!pedal_values.brake_pressed) {
		new_state = VCU_STATE_RTD_RDY;
		return new_state;
	}

	if (state_machine->state_counter == 0) {
		press_start = 0;
	}

	if (rtd_get_btn()) {
		// button pressed
		if (press_start == 0) {
			press_start = osKernelGetTickCount();
		}
		else {
			// hold button for 2s
			if ((osKernelGetTickCount() - press_start) > 2000) {
				new_state = VCU_STATE_DRIVING;
				return new_state;
			}
		}
	}
	else {
		press_start = 0;
	}

	return new_state;
}

VCU_STATE state_driving(state_machine_t *state_machine) {
	VCU_STATE new_state = VCU_STATE_DRIVING;

	bool inverter_good = true;

	for (uint8_t i = 0; i < MCISO_COUNT; i++) {
		// check MCISO board is good
		inverter_good = inverter_good && heartbeat_states.hb_MCISO[i].hb_alive;

		// check connected inverters are good
		inverter_good = inverter_good
				&& heartbeat_boards.MCISO_hbState[i].errorFlags.HB_INV0 == 0;
		inverter_good = inverter_good
				&& heartbeat_boards.MCISO_hbState[i].errorFlags.HB_INV1 == 0;
	}

	if (!inverter_good) {
		// missing inverter, go to error state
		new_state = VCU_STATE_TS_ERROR;
		return new_state;
	}

	// inverters are good, set torque demand based on pedal values / state

	// grab pedal data
	pedal_values_t pedal_values;
	sensor_pedalBox_getData(&pedal_values);

	bool disable_motor = pedal_values.APPS_disable_motors;
	disable_motor = disable_motor || pedal_values.pedal_disable_motors;
	disable_motor = disable_motor || pedal_values.BSE_disable_motors;

	float current_request = 0;

	// get smallest pedal value
	uint16_t pedal_mapped = 0;
	if (pedal_values.pedal_accel_mapped[0]
			< pedal_values.pedal_accel_mapped[1]) {
		pedal_mapped = pedal_values.pedal_accel_mapped[0];
	}
	else {
		pedal_mapped = pedal_values.pedal_accel_mapped[1];
	}

	current_request = pedal_mapped * inverters.max_current;

	if (pedal_mapped < 100) {
		// pedal deadzone
		current_request = 0;
	}

	for (uint8_t i = 0; i < NUM_VESC; i++) {
		inverters.vesc[i].current_request = current_request;
	}

	return new_state;
}

VCU_STATE state_ts_error(state_machine_t *state_machine) {
	VCU_STATE new_state = VCU_STATE_TS_ERROR;

	for (uint8_t i = 0; i < NUM_VESC; i++) {
		inverters.vesc[i].current_request = 0;
	}

	return new_state;
}

#endif
