/*
 * state_machine_states_cool.c
 *
 *  Created on: Nov 17, 2023
 *      Author: Calvin
 */

#include "state_machine.h"
#include "heartbeat.h"

#if (VCU_CURRENT_ID == VCU_ID_COOL_L) || (VCU_CURRENT_ID == VCU_ID_COOL_R)

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

	new_state = VCU_STATE_COOL;

	return new_state;
}

VCU_STATE state_cool(state_machine_t *state_machine) {
	VCU_STATE new_state = VCU_STATE_COOL;

	return new_state;
}

#endif

