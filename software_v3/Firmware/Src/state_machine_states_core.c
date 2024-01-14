/*
 * state_machine_states_core.c
 *
 *  Created on: Oct 31, 2023
 *      Author: Calvin
 */

#include "state_machine.h"
#include "inv_sevcon.h"

VCU_STATE state_start(state_machine_t *state_machine) {
	VCU_STATE new_state = VCU_STATE_START;

	new_state = VCU_STATE_PERIPHERAL_INIT;

	return new_state;
}

VCU_STATE state_shutdown(state_machine_t *state_machine) {
	VCU_STATE new_state = VCU_STATE_SHUTDOWN;

#if (QEV4 == 1) && (VCU_CURRENT_ID == VCU_ID_CTRL)
	for (uint8_t i = 0; i < NUM_INV; i++) {
		// just make sure they're in shutdown
		inverters.sevcon[i].ctrl.torque_demand = 0;
		inverters.sevcon[i].ctrl.control_word = SEVCON_CMD_SHUTDOWN;
	}
#endif

	return new_state;
}

VCU_STATE state_error(state_machine_t *state_machine) {
	VCU_STATE new_state = VCU_STATE_ERROR;

	return new_state;
}
