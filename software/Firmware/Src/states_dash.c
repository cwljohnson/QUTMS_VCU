/*
 * states_dash.c
 *
 *  Created on: 17 Feb. 2022
 *      Author: Calvin J
 */

#include "main.h"
#include "can.h"
#include "states.h"
#include "shutdown.h"
#include "heartbeat.h"
#include "s_suspensionTravel.h"
#include "s_dashLED.h"
#include "s_steeringAngle.h"
#include "12vSW.h"

#include "s_pedalBox.h"
#include <QUTMS_CAN.h>
#include <CAN_BMU.h>

#if VCU_CURRENT_ID == VCU_ID_DASH

state_t state_sInit = { &state_sInit_enter, &state_sInit_body, VCU_STATE_SENSOR_INIT };
state_t state_boardCheck = { &state_boardCheck_enter, &state_boardCheck_body, VCU_STATE_BOARD_CHECK };
state_t state_dash = { &state_dash_enter, &state_dash_body, VCU_STATE_DASH };

uint32_t sensor_retry_start = 0;
uint32_t sensor_timeout_start = 0;

bool AMS_dash_latched;
uint16_t brake_reading;

void state_sInit_enter(fsm_t *fsm) {
	setup_dashLED();
	AMS_dash_latched = false;

	//setup_susTravel();
	setup_steeringAngle();

	bool success = true;

	if (!check_steeringAngle_connected()) {
		success = false;
	}
	/*
	if (!check_susTravel_connected()) {
		success = false;
	}
*/
	if (!success) {
		// sensors failed, start timers so we can retry
		sensor_retry_start = HAL_GetTick();
		sensor_timeout_start = HAL_GetTick();
	}
	else {
		// sensors good
		fsm_changeState(fsm, &state_boardCheck, "Sensors initialized");
		sensor_retry_start = 0;
		sensor_timeout_start = 0;
		return;
	}
}

void state_sInit_body(fsm_t *fsm) {
	if (sensor_timeout_start == 0) {
		// everything is connected so skip this iteration
		return;
	}

	if ((sensor_retry_start - HAL_GetTick()) > SENSOR_RETRY) {
		bool success = true;

		if (!check_steeringAngle_connected()) {
			success = false;
		}
/*
		if (!check_susTravel_connected()) {
			success = false;
		}
*/
		if (!success) {
			// sensor failed

			sensor_retry_start = HAL_GetTick();
		}
		else {
			// sensors good, so move to board check

			// disable timeout
			sensor_timeout_start = 0;

			fsm_changeState(fsm, &state_boardCheck, "Sensors initialized");
			return;
		}
	}

	if ((sensor_timeout_start - HAL_GetTick()) > SENSOR_TIMEOUT) {
		// sensor has failed so go to error state
		fsm_changeState(fsm, &state_error, "Sensors failed");
		return;
	}
}

void state_boardCheck_enter(fsm_t *fsm) {

}

void state_boardCheck_body(fsm_t *fsm) {
	// if all heartbeats / boards are present go to check BMU

	CAN_MSG_Generic_t msg;
	while (queue_next(&queue_CAN, &msg)) {
		// check for heartbeats
		check_heartbeat_msg(&msg);
	}

	// update state of heartbeat error flags
	// don't need to go into an error state if boards aren't present
	// so won't check return value
	check_bad_heartbeat();

	bool boards_missing = false;

	if (!heartbeats.VCU[VCU_ID_SHDN]) {
		//boards_missing = true;
	}

	if (!boards_missing) {
		// all boards required are present

		fsm_changeState(fsm, &state_dash, "SHDN VCU present");
		return;
	}
}

bool status;

uint32_t latch_timeout_start = 0;

void state_dash_enter(fsm_t *fsm) {
	status = false;

	SW_hbState.flags.rawMem = 0;
/*
	if (SW_hbState.flags._SW_Flags.FAN_ENABLE == 1) {
		// turn fan on
		SW_setState(0, true);
	}
	else {
		// turn fan off
		SW_setState(0, false);
	}
*/

	latch_timeout_start = HAL_GetTick();

	return;
}



void state_dash_body(fsm_t *fsm) {
	CAN_MSG_Generic_t msg;

	uint8_t segments[4];

	bool new_can = false;
	bool new_bmu = false;

	while (queue_next(&queue_CAN, &msg)) {
		// check for heartbeat
		if (check_heartbeat_msg(&msg)) {
			if (msg.ID == BMU_Heartbeat_ID) {
				new_bmu = true;
			} else if (msg.ID == SW_Heartbeat_ID) {
				SW_hbState.flags.rawMem = SW_hbState.flags.rawMem;
			}
		}

		// check for shutdowns
		else if (msg.ID == VCU_ShutdownStatus_ID) {
			Parse_VCU_ShutdownStatus(msg.data, &segments[0], &segments[1], &segments[2], &segments[3], &status);
			new_can = true;
		}
		else if (msg.ID == VCU_Pedal_Brake_ID) {
			uint16_t dummy;
			Parse_VCU_Pedal_Brake(msg.data, &brake_reading, &dummy, &dummy);
		}

	}

	if (SW_hbState.flags._SW_Flags.FAN_ENABLE == 1) {
		// turn fan on
		SW_setState(0, true);
	}
	else {
		// turn fan off
		SW_setState(0, false);
	}


	if (brake_reading > BRAKE_MIN_ACTUATION) {
		SW_setState(1, true);
		VCU_heartbeatState.otherFlags.dash._VCU_Flags_Dash.BRAKE_LIGHT = 1;
	}
	else {
		SW_setState(1, false);
		VCU_heartbeatState.otherFlags.dash._VCU_Flags_Dash.BRAKE_LIGHT = 0;
	}

#if DASH_DEBUG_SHDN == 1
	// forces shutdown line status to be good
	status = true;
#endif

	bool latch_bypass = false;

	if ((HAL_GetTick() - latch_timeout_start) < DASH_LATCH_BYPASS_TIME) {
		// car has been on for less than whatever seconds, bypass LED latching
		latch_bypass = true;
	} else {
		// car has been on for more than whatever, latching do your thang
		latch_bypass = false;
	}

	// check for BMU faults
	if ((BMU_hbState.flags._BMU_Flags.SHDN_BMU == 1) || BMU_hbState.stateID == BMU_STATE_ERROR) {
		// BMU in error state, latch dash light on

		AMS_dash_latched = true;
		dashLED_setState(DASH_CH_LED_AMS, true);
	}
	else {
		if (status || latch_bypass) {
			// BMU is good, and shutdown system is good, so unlatch and turn off dash light
			AMS_dash_latched = false;
			dashLED_setState(DASH_CH_LED_AMS, false);
		}
	}

	if (!check_bad_heartbeat()) {
		// something has dropped out

		if (!heartbeats.BMU) {
			// BMU missing, turn dash on
			dashLED_setState(DASH_CH_LED_AMS, true);
		}
	}
	else {
		if (!AMS_dash_latched && heartbeats.BMU) {
			// BMU has recovered, and dash isn't latched on, so turn dash off
			dashLED_setState(DASH_CH_LED_AMS, false);
		}
	}

	if (new_can) {
		bool bspd = ((segments[1] >> 2) & 0x1) == 1;

		if (bspd) {
			dashLED_setState(DASH_CH_LED_BSPD, true);
		}
		else if (status || latch_bypass) {
			// shutdown is good, so reset
			dashLED_setState(DASH_CH_LED_BSPD, false);
		}
	}

	if (new_bmu) {
		bool imd = BMU_hbState.flags._BMU_Flags.SHDN_IMD == 1;
		bool pdoc = BMU_hbState.flags._BMU_Flags.SHDN_PDOC == 1;

		if (pdoc) {
			dashLED_setState(DASH_CH_LED_PDOC, true);
		}
		else if (status || latch_bypass) {
			// shutdown is good, so reset
			dashLED_setState(DASH_CH_LED_PDOC, false);
		}

		if (imd) {
			dashLED_setState(DASH_CH_LED_IMD, true);
		}
		else if (status || latch_bypass) {
			// shutdown is good, so reset
			dashLED_setState(DASH_CH_LED_IMD, false);
		}
	}
}

#endif
