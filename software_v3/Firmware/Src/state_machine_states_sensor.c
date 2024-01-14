/*
 * state_machine_states_sensor.c
 *
 *  Created on: Dec 11, 2023
 *      Author: Calvin
 */

#include "state_machine.h"
#include "state_machine_states.h"
#include "heartbeat.h"

#if (VCU_CURRENT_ID == VCU_ID_SENSOR)

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

	new_state = VCU_STATE_SENSOR;

	return new_state;
}

VCU_STATE state_sensor(state_machine_t *state_machine) {
	VCU_STATE new_state = VCU_STATE_SENSOR;

	return new_state;
}

#endif

