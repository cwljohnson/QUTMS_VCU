/*
 * state_machine_states_ctrl_qev3_dvl.c
 *
 *  Created on: Dec 12, 2023
 *      Author: Calvin
 */

#include "state_machine.h"

#include "heartbeat.h"
#include "sensor_rtd.h"
#include "sensor_pedalBox.h"
#include "inv_vesc.h"

#if QEV3 == 1
#if DRIVERLESS_CTRL == 1

VCU_STATE state_dvl_ebs_check(state_machine_t *state_machine) {
	VCU_STATE new_state = VCU_STATE_DVL_EBS_CHECK;

	bool EBS_present = false;

	if (heartbeat_states.hb_EBS_CTRL.hb_alive) {
		EBS_present = true;
	}

	if (EBS_present) {

		if (heartbeat_boards.EBS_CTRL_hbState.stateID
				== EBS_CTRL_STATE_CTRL_ACK) {
			new_state = VCU_STATE_DVL_RQST_MISSION;
			return new_state;
		}
	}

	return new_state;
}

VCU_STATE state_dvl_rqst_mission(state_machine_t *state_machine) {
	VCU_STATE new_state = VCU_STATE_DVL_RQST_MISSION;

	if (heartbeat_states.hb_DVL.hb_alive) {
		if (heartbeat_boards.DVL_hbState.missionID == DVL_MISSION_MANUAL) {
			new_state = VCU_STATE_IDLE;
			return new_state;
		}
		else if (heartbeat_boards.DVL_hbState.missionID
				== DVL_MISSION_SELECTED) {
			new_state = VCU_STATE_DVL_IDLE;
			return new_state;
		}
	}

	return new_state;
}

VCU_STATE state_dvl_idle(state_machine_t *state_machine) {
	VCU_STATE new_state = VCU_STATE_DVL_IDLE;

	static uint32_t counter = 0;

	if (state_machine->state_counter == 0) {
		// clear counter if first call
		counter = 0;
	}

	if ((heartbeat_states.hb_VCU_EBS_BTN.hb_alive)
			&& (heartbeat_boards.VCU_EBS_BTN_hbState.otherFlags.ebs_btn._VCU_Flags_EBS_BTN.BTN_PRESSED
					== 1)) {
		if (counter == 0) {
			counter = state_machine->current_ticks;
		}
		else {
			if ((state_machine->current_ticks - counter) > 1000U) {
				new_state = VCU_STATE_DVL_PRECHARGE_REQUEST;
				return new_state;
			}
		}
	}
	else {
		counter = 0;
	}

	return new_state;
}

VCU_STATE state_dvl_precharge_request(state_machine_t *state_machine) {
	VCU_STATE new_state = VCU_STATE_DVL_PRECHARGE_REQUEST;

	if ((heartbeat_boards.BMU_hbState.stateID == BMU_STATE_PRECHARGE)
			|| (heartbeat_boards.BMU_hbState.stateID == BMU_STATE_TS_ACTIVE)) {
		new_state = VCU_STATE_DVL_PRECHARGE;
		return new_state;
	}
	else if (heartbeat_boards.BMU_hbState.stateID != BMU_STATE_READY) {
		// if it's in ready, probably just about to start precharge so ignore
		// any other state is an error

		// something has clowned, so go back to BMU health check
		new_state = VCU_STATE_BMU_CHECK;
		return new_state;
	}

	return new_state;
}

VCU_STATE state_dvl_precharge(state_machine_t *state_machine) {
	VCU_STATE new_state = VCU_STATE_DVL_PRECHARGE;

	if (heartbeat_boards.BMU_hbState.stateID == BMU_STATE_TS_ACTIVE) {
		// precharge finished successfully, TS is active
		new_state = VCU_STATE_DVL_INVERTER_CHECK;
		return new_state;
	}
	else if ((heartbeat_boards.BMU_hbState.stateID == BMU_STATE_READY)
			&& (heartbeat_boards.BMU_hbState.flags._BMU_Flags.PCHRG_TIMEOUT == 1)) {
		// precharge timed out
		// go back to idle and start again

		new_state = VCU_STATE_DVL_IDLE;
		return new_state;
	}
	else if (heartbeat_boards.BMU_hbState.stateID != BMU_STATE_PRECHARGE) {
		// why tf we go back, something is broken
		// go to BMU health check

		// something has clowned, so go back to BMU health check
		new_state = VCU_STATE_BMU_CHECK;
		return new_state;
	}

	return new_state;
}

VCU_STATE state_dvl_inverter_check(state_machine_t *state_machine) {
	VCU_STATE new_state = VCU_STATE_DVL_INVERTER_CHECK;

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
		new_state = VCU_STATE_DVL_RTD;
		return new_state;
	}

	return new_state;
}

VCU_STATE state_dvl_rtd(state_machine_t *state_machine) {
	VCU_STATE new_state = VCU_STATE_DVL_RTD;

	if (heartbeat_boards.DVL_hbState.stateID == DVL_STATE_DRIVING) {
		// everything good lets drive
		new_state = VCU_STATE_DVL_DRIVING;
		return new_state;
	}

	return new_state;
}

VCU_STATE state_dvl_driving(state_machine_t *state_machine) {
	VCU_STATE new_state = VCU_STATE_DVL_DRIVING;

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

	if (heartbeat_boards.DVL_hbState.stateID == DVL_STATE_EMERGENCY) {
		new_state = VCU_STATE_DVL_EMERGENCY;
		return new_state;
	}

	bool disable_motor = false;

	// grab pedal data
	pedal_values_t pedal_values;
	sensor_pedalBox_getData(&pedal_values);

	disable_motor = disable_motor || pedal_values.BSE_disable_motors;

	double current_accel = heartbeat_boards.DVL_hbState.torqueRequest / 100.0;

	if (!heartbeat_states.hb_DVL.hb_alive) {
		// DVL supervisor dropped out
		current_accel = 0;
	}

	if (heartbeat_boards.RES_hbState.estop) {
		// RES has been pressed
		current_accel = 0;
	}

	float current_request = 0;

	if (current_accel > 0) {
		current_request = current_accel * inverters.max_current;
	}
	else if (current_accel < 0) {
		current_request = current_accel * inverters.max_regen_current;
	}

	for (uint8_t i = 0; i < NUM_VESC; i++) {
		inverters.vesc[i].current_request = current_request;
	}

	return new_state;
}

VCU_STATE state_dvl_emergency(state_machine_t *state_machine) {
	VCU_STATE new_state = VCU_STATE_DVL_EMERGENCY;

	return new_state;
}

#endif
#endif
