/*
 * sensor_brakelight.c
 *
 *  Created on: Nov 6, 2023
 *      Author: Calvin
 */

#include "sensor_brakelight.h"
#include "heartbeat.h"
#include "p_12vSW.h"

void sensor_brakelight_update_outputs(state_machine_t *state_machine) {
	// this is a bit naughty because this should really just be based on state, but oh well
	// can assume heartbeat mutex is locked

	bool activated = false;

	if ((heartbeat_states.hb_VCU_CTRL.hb_alive) && (heartbeat_boards.VCU_CTRL_hbState.otherFlags.ctrl._VCU_Flags_Ctrl.Brake_Pressed == 1)) {
		activated = true;
	}

#if (VCU_CURRENT_ID == VCU_ID_ACCU)
	VCU_heartbeatState.otherFlags.accu._VCU_Flags_ACCU.BRAKE_LIGHT = activated ? 1 : 0;
#endif

#if (VCU_CURRENT_ID == VCU_ID_DASH)
	VCU_heartbeatState.otherFlags.dash._VCU_Flags_Dash.BRAKE_LIGHT = activated ? 1 : 0;
#endif

	SW_setState(SW_IDX_P_BRAKELIGHT, activated);
}
