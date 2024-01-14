/*
 * state_machine_states_ebs_btn.c
 *
 *  Created on: Dec 15, 2023
 *      Author: Calvin
 */

#include "state_machine.h"
#include "state_machine_states.h"

#if VCU_CURRENT_ID == VCU_ID_EBS_BTN

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

	new_state = VCU_STATE_EBS_BTN;

	return new_state;
}

VCU_STATE state_ebs_btn(state_machine_t *state_machine) {
	VCU_STATE new_state = VCU_STATE_EBS_BTN;

	return new_state;
}

#endif
