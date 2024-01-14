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
#include "12vSW.h"
#include <Timer.h>

#include <stdbool.h>

#include <QUTMS_CAN.h>
#include <CAN_DVL.h>
#include <CAN_VCU.h>

#if VCU_CURRENT_ID == VCU_ID_ASSI

state_t state_sInit = { &state_sInit_enter, &state_sInit_body, VCU_STATE_SENSOR_INIT };
state_t state_boardCheck = { &state_boardCheck_enter, &state_boardCheck_body, VCU_STATE_BOARD_CHECK };
state_t state_assi = { &state_assi_enter, &state_assi_body, VCU_STATE_ASSI };

uint32_t sensor_retry_start = 0;
uint32_t sensor_timeout_start = 0;

ms_timer_t timer_assi;

uint8_t AS_state;
void assi_cb(void *args);

bool toggleState = false;

void state_sInit_enter(fsm_t *fsm) {
	bool success = true;

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

	if (!boards_missing) {
		// all boards required are present

		fsm_changeState(fsm, &state_assi, "Everything good");
		return;
	}
}

void state_assi_enter(fsm_t *fsm) {
	timer_assi = timer_init(50, true, assi_cb);
	timer_start(&timer_assi);

	AS_state = AS_state_off;

	return;
}

void state_assi_body(fsm_t *fsm) {
	CAN_MSG_Generic_t msg;

	while (queue_next(&queue_CAN, &msg)) {
		// check for heartbeat
		if (check_heartbeat_msg(&msg)) {

		}
	}

	timer_update(&timer_assi, NULL);
}

void assi_cb(void *args) {
	switch (DVL_hbState.stateID) {
	case DVL_STATE_START:
	case DVL_STATE_SELECT_MISSION:
	case DVL_STATE_WAIT_FOR_MISSION:
	case DVL_STATE_CHECK_EBS:
		AS_state = AS_state_off;
		break;

	case DVL_STATE_READY:
		AS_state = AS_state_ready;
		break;

	case DVL_STATE_RELEASE_EBS:
	case DVL_STATE_DRIVING:
		AS_state = AS_state_driving;
		break;

	case DVL_STATE_ACTIVATE_EBS:
	case DVL_STATE_FINISHED:
		AS_state = AS_state_finish;
		break;

	case DVL_STATE_EMERGENCY:
		AS_state = AS_state_emergency_brake;
		break;

	default:
		AS_state = AS_state_off;
		break;
	}

	VCU_heartbeatState.otherFlags.assi._VCU_Flags_ASSI.AS_STATE = AS_state;

	toggleState = !toggleState;

	if (AS_state == AS_state_off) {
		SW_setState(0, false); // yellow
		SW_setState(1, false); // blue
	}
	else if (AS_state == AS_state_ready) {
		SW_setState(0, true); // yellow
		SW_setState(1, false); // blue
	}
	else if (AS_state == AS_state_driving) {
		SW_setState(0, toggleState); // yellow
		SW_setState(1, false); // blue
	}
	else if (AS_state == AS_state_finish) {
		SW_setState(0, false); // yellow
		SW_setState(1, true); // blue
	}
	else if (AS_state == AS_state_emergency_brake) {
		SW_setState(0, false); // yellow
		SW_setState(1, toggleState); // blue
	}

}

#endif
