/*
 * state_machine_states_ctrl.c
 *
 *  Created on: Sep 5, 2023
 *      Author: Calvin
 */

#include "state_machine.h"
#include "heartbeat.h"

#include "sensor_rtd.h"

#include <CAN_BMU.h>


#if VCU_CURRENT_ID == VCU_ID_CTRL

VCU_STATE state_peripheral_init(state_machine_t *state_machine) {
	VCU_STATE new_state = VCU_STATE_PERIPHERAL_INIT;

	new_state = VCU_STATE_SENSOR_INIT;

	return new_state;
}

VCU_STATE state_sensor_init(state_machine_t *state_machine) {
	VCU_STATE new_state = VCU_STATE_SENSOR_INIT;

	new_state = VCU_STATE_BOARD_CHECK;

	return new_state;
}

VCU_STATE state_board_check(state_machine_t *state_machine) {
	VCU_STATE new_state = VCU_STATE_BOARD_CHECK;

	bool boards_missing = false;

#if DEBUG_BMU == 0
	// BMU is critical
	if (!heartbeat_states.hb_BMU.hb_alive) {
		boards_missing = true;
	}
#endif

	if (!boards_missing) {
		new_state = VCU_STATE_BMU_CHECK;
		return new_state;
	}

	return new_state;
}

VCU_STATE state_bmu_check(state_machine_t *state_machine) {
	VCU_STATE new_state = VCU_STATE_BMU_CHECK;

#if DEBUG_BMU == 0
	// BMU is critical
	if (heartbeat_boards.BMU_hbState.stateID == BMU_STATE_READY) {

#if (QEV3 == 1) && (DRIVERLESS_CTRL == 1)
		new_state = VCU_STATE_DVL_EBS_CHECK;
#else
		new_state = VCU_STATE_IDLE;
#endif

		return new_state;
	}
#endif

	return new_state;
}

VCU_STATE state_idle(state_machine_t *state_machine) {
	static uint32_t counter = 0;

	VCU_STATE new_state = VCU_STATE_IDLE;

	if (state_machine->state_counter == 0) {
		// clear counter if first call
		counter = 0;
	}

#if DEBUG_BMU == 0
	if (heartbeat_boards.BMU_hbState.stateID != BMU_STATE_READY) {
		new_state = VCU_STATE_BMU_CHECK;
		return new_state;
	}
#endif

	if (rtd_get_btn()) {
		if (counter == 0) {
			counter = state_machine->current_ticks;
		}
		else {
			if ((state_machine->current_ticks - counter) > 1000U) {
				new_state = VCU_STATE_PRECHARGE_REQUEST;
				return new_state;
			}
		}
	}
	else {
		counter = 0;
	}

	return new_state;
}

VCU_STATE state_precharge_request(state_machine_t *state_machine) {
	VCU_STATE new_state = VCU_STATE_PRECHARGE_REQUEST;

	if ((heartbeat_boards.BMU_hbState.stateID == BMU_STATE_PRECHARGE)
			|| (heartbeat_boards.BMU_hbState.stateID == BMU_STATE_TS_ACTIVE)) {
		// precharge request has been acknowledged and started, or it's already finished so move to precharge to confirm and wait
		new_state = VCU_STATE_PRECHARGE;
		return new_state;
	}
	else if ((heartbeat_boards.BMU_hbState.stateID != BMU_STATE_READY)) {
		// if it's in ready, probably just about to start precharge so ignore
		// any other state is an error

		// something has clowned, so go back to BMU health check
		new_state = VCU_STATE_BMU_CHECK;
		return new_state;
	}

	return new_state;
}

VCU_STATE state_precharge(state_machine_t *state_machine) {
	VCU_STATE new_state = VCU_STATE_PRECHARGE;

	if (heartbeat_boards.BMU_hbState.stateID == BMU_STATE_TS_ACTIVE) {
		// precharge finished successfully, TS is active
		// go to inverter health check
		new_state = VCU_STATE_INVERTER_CHECK;
		return new_state;
	}
	else if ((heartbeat_boards.BMU_hbState.stateID == BMU_STATE_READY)
			&& (heartbeat_boards.BMU_hbState.flags._BMU_Flags.PCHRG_TIMEOUT == 1)) {
		// precharge timed out
		// go back to idle and start again

		new_state = VCU_STATE_IDLE;
		return new_state;
	}
	else if (heartbeat_boards.BMU_hbState.stateID != BMU_STATE_PRECHARGE) {
		// why tf we go back, something is broken
		// go to BMU health check
		new_state = VCU_STATE_BMU_CHECK;
		return new_state;
	}

	return new_state;
}

#endif
