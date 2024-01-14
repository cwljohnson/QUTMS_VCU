/*
 * state_machine.h
 *
 *  Created on: Sep 5, 2023
 *      Author: Calvin
 */

#ifndef INC_STATE_MACHINE_H_
#define INC_STATE_MACHINE_H_

#include <CAN_VCU.h>

#include <stdint.h>
#include "can_rtos.h"
#include "sensor_shutdown.h"

typedef struct {
	VCU_STATE state_current;
	uint32_t state_counter;
	uint32_t enter_ticks;
	uint32_t current_ticks;

	shutdown_status_t shutdown_status;
} state_machine_t;

void state_machine_init(state_machine_t *state_machine);
VCU_STATE state_machine_update(state_machine_t *state_machine);
VCU_STATE state_machine_change_state(state_machine_t *state_machine, VCU_STATE new_state);

VCU_STATE state_machine_handle_CAN(state_machine_t *state_machine, can_msg_t *msg);

void state_machine_set_outputs(state_machine_t *state_machine);

#endif /* INC_STATE_MACHINE_H_ */
