/*
 * sensor_rear_fan.c
 *
 *  Created on: Dec 9, 2023
 *      Author: Calvin
 */

#include "sensor_rear_fan.h"
#include "p_12vSW.h"
#include "heartbeat.h"

void sensor_rear_fan_update_outputs(state_machine_t *state_machine) {
	bool enable = (state_machine->state_current == VCU_STATE_ACCU);

	SW_setPWM(SW_IDX_P_REAR_FAN, 5);


	VCU_heartbeatState.otherFlags.accu._VCU_Flags_ACCU.FAN_REAR = enable ? 1 : 0;
}

