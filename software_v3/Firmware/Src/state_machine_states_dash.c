/*
 * state_machine_states_dash.c
 *
 *  Created on: Dec 4, 2023
 *      Author: Calvin
 */

#include "state_machine.h"
#include "heartbeat.h"
#include "sensor_dashLED.h"
#include <CAN_BMU.h>

#if VCU_CURRENT_ID == VCU_ID_DASH

bool AMS_dash_latched = false;

VCU_STATE state_peripheral_init(state_machine_t *state_machine) {
	VCU_STATE new_state = VCU_STATE_PERIPHERAL_INIT;

	new_state = VCU_STATE_SENSOR_INIT;

	return new_state;
}

VCU_STATE state_sensor_init(state_machine_t *state_machine) {
	VCU_STATE new_state = VCU_STATE_SENSOR_INIT;

	AMS_dash_latched = false;

	new_state = VCU_STATE_BOARD_CHECK;

	return new_state;
}

VCU_STATE state_board_check(state_machine_t *state_machine) {
	VCU_STATE new_state = VCU_STATE_BOARD_CHECK;

	new_state = VCU_STATE_DASH;

	return new_state;
}

VCU_STATE state_dash(state_machine_t *state_machine) {
	VCU_STATE new_state = VCU_STATE_DASH;

	bool latch_bypass = (state_machine->current_ticks < DASH_LATCH_BYPASS_TIME);

	if ((heartbeat_boards.BMU_hbState.flags._BMU_Flags.SHDN_BMU == 1)
			|| (heartbeat_boards.BMU_hbState.stateID == BMU_STATE_ERROR)) {
		AMS_dash_latched = true;
		sensor_dashLED_setState(DASH_CH_LED_AMS, true);
	}
	else {
		if (state_machine->shutdown_status.state || latch_bypass) {
			// BMU is good, and shutdown system is good, so unlatch and turn off dash light
			AMS_dash_latched = false;
			sensor_dashLED_setState(DASH_CH_LED_AMS, false);
		}
	}

	if (!heartbeat_states.hb_BMU.hb_alive) {
		// bmu missing, turn dash on
		sensor_dashLED_setState(DASH_CH_LED_AMS, true);
	}
	else {
		if (!AMS_dash_latched) {
			// BMU has recovered, and dash isn't latched on, so turn dash off
			sensor_dashLED_setState(DASH_CH_LED_AMS, false);
		}
	}

	bool bspd = ((state_machine->shutdown_status.segs[1] >> 2) & 0x1) != 0;

	if (bspd) {
		sensor_dashLED_setState(DASH_CH_LED_BSPD, true);
	}
	else if (state_machine->shutdown_status.state || latch_bypass) {
		// shutdown is good, so reset
		sensor_dashLED_setState(DASH_CH_LED_BSPD, false);
	}

	bool imd = heartbeat_boards.BMU_hbState.flags._BMU_Flags.SHDN_IMD == 1;
	bool pdoc = heartbeat_boards.BMU_hbState.flags._BMU_Flags.SHDN_PDOC == 1;

	if (imd) {
		sensor_dashLED_setState(DASH_CH_LED_IMD, true);
	}
	else if (state_machine->shutdown_status.state || latch_bypass) {
		// shutdown is good, so reset
		sensor_dashLED_setState(DASH_CH_LED_IMD, false);
	}

	if (pdoc) {
		sensor_dashLED_setState(DASH_CH_LED_PDOC, true);
	}
	else if (state_machine->shutdown_status.state || latch_bypass) {
		// shutdown is good, so reset
		sensor_dashLED_setState(DASH_CH_LED_PDOC, false);
	}

	return new_state;
}

#endif
