/*
 * state_machine_states_ctrl_qev4.c
 *
 *  Created on: Nov 23, 2023
 *      Author: Calvin
 */

#include "state_machine.h"

#include "heartbeat.h"
#include "sensor_rtd.h"
#include "inv_sevcon.h"
#include "sensor_pedalBox.h"

#if QEV4 == 1

VCU_STATE state_inverter_check(state_machine_t *state_machine) {
	VCU_STATE new_state = VCU_STATE_INVERTER_CHECK;

	bool ready = true;
	bool inverter_fault = false;

	for (uint8_t i = 0; i < NUM_INV; i++) {
		inverters.sevcon[i].ctrl.control_word = SEVCON_CMD_SHUTDOWN;

		if (!inverters.sevcon[i].alive) {
			ready = false;
		}

		sevcon_state_t status_word = inverters.sevcon[i].data.status_word;

		if ((status_word == SEVCON_STATE_FAULT_ACTIVE)
				|| (status_word == SEVCON_STATE_FAULT_OFF)) {
			inverter_fault = true;
		}

		// wait for inverters to be in correct state
		if (status_word != SEVCON_STATE_SHUTDOWN) {
			ready = false;
		}
	}

	if (inverter_fault) {
		// we can't recover from this, go to error state as car will have to be restarted
		new_state = VCU_STATE_TS_ERROR;
		return new_state;
	}

	if (ready) {
		new_state = VCU_STATE_INVERTER_ENERGIZE;
		return new_state;
	}

	return new_state;
}

VCU_STATE state_inverter_energize(state_machine_t *state_machine) {
	VCU_STATE new_state = VCU_STATE_INVERTER_ENERGIZE;

	bool ready = true;
	bool bad_status = false;

	for (uint8_t i = 0; i < NUM_INV; i++) {
		inverters.sevcon[i].ctrl.control_word = SEVCON_CMD_ENERGISE;

		if (!inverters.sevcon[i].alive) {
			ready = false;
		}

		sevcon_state_t status_word = inverters.sevcon[i].data.status_word;

		// wait for inverters to be in correct state
		if (status_word != SEVCON_STATE_ENERGISED) {
			ready = false;
		}

		// wrong state, go back
		if (!((status_word == SEVCON_STATE_SHUTDOWN)
				|| (status_word == SEVCON_STATE_PRECHARGE)
				|| (status_word == SEVCON_STATE_ENERGISED))) {
			bad_status = true;
		}
	}

	if (bad_status) {
		// an inverter is not in a valid state, go back to check
		new_state = VCU_STATE_INVERTER_CHECK;
		return new_state;
	}

	if (ready) {
		new_state = VCU_STATE_RTD_RDY;
		return new_state;
	}

	return new_state;
}

VCU_STATE state_rtd_rdy(state_machine_t *state_machine) {
	VCU_STATE new_state = VCU_STATE_RTD_RDY;

	bool bad_status = false;

	for (uint8_t i = 0; i < NUM_INV; i++) {
		inverters.sevcon[i].ctrl.control_word = SEVCON_CMD_ENERGISE;

		if (!inverters.sevcon[i].alive) {
			// inverter dropped out, go back
			bad_status = true;
		}

		sevcon_state_t status_word = inverters.sevcon[i].data.status_word;

		// wrong state, go back
		if (status_word != SEVCON_STATE_ENERGISED) {
			bad_status = true;
		}
	}

	if (bad_status) {
		// an inverter is not in a valid state, go back to check
		new_state = VCU_STATE_INVERTER_CHECK;
		return new_state;
	}

	pedal_values_t pedal_values;
	sensor_pedalBox_getData(&pedal_values);

#if RTD_DEBUG == 0
	// brake has to be pressed
	if (pedal_values.brake_pressed) {
		new_state = VCU_STATE_RTD_BTN;
		return new_state;
	}
#else
	new_state = VCU_STATE_RTD_BTN;
#endif

	return new_state;
}

VCU_STATE state_rtd_btn(state_machine_t *state_machine) {
	VCU_STATE new_state = VCU_STATE_RTD_BTN;

	bool bad_status = false;

	for (uint8_t i = 0; i < NUM_INV; i++) {
		inverters.sevcon[i].ctrl.control_word = SEVCON_CMD_ENERGISE;

		if (!inverters.sevcon[i].alive) {
			// inverter dropped out, go back
			bad_status = true;
		}

		sevcon_state_t status_word = inverters.sevcon[i].data.status_word;

		// wrong state, go back
		if (status_word != SEVCON_STATE_ENERGISED) {
			bad_status = true;
		}
	}

	if (bad_status) {
		// an inverter is not in a valid state, go back to check
		new_state = VCU_STATE_INVERTER_CHECK;
		return new_state;
	}

	static uint32_t press_start = 0;

	pedal_values_t pedal_values;
	sensor_pedalBox_getData(&pedal_values);

	// if brake isn't pressed go back lol
#if RTD_DEBUG == 0
	if (!pedal_values.brake_pressed) {
		new_state = VCU_STATE_RTD_RDY;
		return new_state;
	}
#endif

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

	bool bad_status = false;
	bool enabled = true;
	bool inverter_fault = false;

	for (uint8_t i = 0; i < NUM_INV; i++) {
		inverters.sevcon[i].ctrl.control_word = SEVCON_CMD_ENABLE;

		if (!inverters.sevcon[i].alive) {
			// inverter dropped out, go back
			bad_status = true;
		}

		sevcon_state_t status_word = inverters.sevcon[i].data.status_word;

		if ((status_word == SEVCON_STATE_FAULT_ACTIVE)
				|| (status_word == SEVCON_STATE_FAULT_OFF)) {
			inverter_fault = true;
		}

		// check if we're
		if (!((status_word == SEVCON_STATE_ENERGISED)
				|| (status_word == SEVCON_STATE_ENABLED))) {
			bad_status = true;
		}
		else {
			// in energised or enabled
			if (status_word != SEVCON_STATE_ENABLED) {
				enabled = false;
			}
		}
	}

	if (inverter_fault) {
		// we can't recover from this, go to error state as car will have to be restarted
		new_state = VCU_STATE_TS_ERROR;
		return new_state;
	}

	if (bad_status) {
		// an inverter is not in a valid state, go back to check
		new_state = VCU_STATE_INVERTER_CHECK;
		return new_state;
	}

	if (!enabled) {
		// not enabled, zero out torque request
		for (uint8_t i = 0; i < NUM_INV; i++) {
			inverters.sevcon[i].ctrl.torque_demand = 0;
		}
	}
	else {
		// inverters are good, set torque demand based on pedal values / state

		// grab pedal data

		pedal_values_t pedal_values;
		sensor_pedalBox_getData(&pedal_values);

		bool disable_motor = pedal_values.APPS_disable_motors;
		disable_motor = disable_motor || pedal_values.pedal_disable_motors;
		disable_motor = disable_motor || pedal_values.BSE_disable_motors;

		float torque_demand = 0;

		// get smallest pedal value
		uint16_t pedal_mapped = 0;
		if (pedal_values.pedal_accel_mapped[0]
				< pedal_values.pedal_accel_mapped[1]) {
			pedal_mapped = pedal_values.pedal_accel_mapped[0];
		}
		else {
			pedal_mapped = pedal_values.pedal_accel_mapped[1];
		}

		// safety deadzone
		if (pedal_mapped > 100) {
			torque_demand = (pedal_mapped * inverters.max_torque)
					/ (float) pedal_settings.pedal_duty_cycle;

		}

		if (disable_motor) {
			torque_demand = 0;
		}

		if (pedal_values.brake_pressed) {
			// if brake is pressed dont try to spin lol
			torque_demand = 0;
		}

		// temporary limit of torque to 10 Nm, so pedal stays nice
//		if (torque_demand > 10) {
//			torque_demand = 10;
//		}

		bool speed_cutoff = false;
		for (uint8_t i = 0; i < NUM_INV; i++) {
			if (inverters.sevcon[i].data.motor_speed > inverters.sevcon[i].ctrl.speed_limit_soft) {
				speed_cutoff = true;
			}
		}

		if (torque_demand > 2) {
			if (speed_cutoff) {
				torque_demand = 2;
			}
		}

		// TODO: regen jazz

		for (uint8_t i = 0; i < NUM_INV; i++) {
			inverters.sevcon[i].ctrl.torque_demand = torque_demand;
		}

	}

	return new_state;
}

VCU_STATE state_ts_error(state_machine_t *state_machine) {
	VCU_STATE new_state = VCU_STATE_TS_ERROR;

	for (uint8_t i = 0; i < NUM_INV; i++) {
		// just make sure they're in shutdown
		inverters.sevcon[i].ctrl.control_word = SEVCON_CMD_SHUTDOWN;
	}

	return new_state;
}

#endif
