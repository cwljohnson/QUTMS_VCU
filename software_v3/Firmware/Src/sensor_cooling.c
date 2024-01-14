/*
 * sensor_cooling.c
 *
 *  Created on: Dec 2, 2023
 *      Author: Calvin
 */

#include "sensor_cooling.h"
#include "p_12vSW.h"
#include "heartbeat.h"

void sensor_cooling_update_outputs(state_machine_t *state_machine) {
	bool enable = (state_machine->state_current == VCU_STATE_COOL);

	SW_setPWM(SW_IDX_P_SIDE_FAN, 5);
	SW_setState(SW_IDX_P_SIDE_PUMP, true);

	VCU_heartbeatState.otherFlags.cool._VCU_Flags_COOL.SIDE_FAN = enable ? 1 : 0;
	VCU_heartbeatState.otherFlags.cool._VCU_Flags_COOL.SIDE_PUMP = enable ? 1 : 0;
}

