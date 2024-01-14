/*
 * states_ctrl_init.c
 *
 *  Created on: Feb 7, 2022
 *      Author: Calvin J
 */

#include "main.h"
#include "can.h"
#include "states.h"
#include "shutdown.h"
#include "heartbeat.h"
#include "s_pedalBox.h"
#include "s_ctrl_steering.h"
#include "p_isrc.h"
#include "rtd.h"
#include "dvl_emergency.h"
#include "s_dashASB.h"


#if VCU_CURRENT_ID == VCU_ID_CTRL

state_t state_sInit = { &state_sInit_enter, &state_sInit_body, VCU_STATE_SENSOR_INIT };
state_t state_boardCheck = { &state_boardCheck_enter, &state_boardCheck_body, VCU_STATE_BOARD_CHECK };
state_t state_checkBMU = { &state_checkBMU_enter, &state_checkBMU_body, VCU_STATE_BMU_CHECK };


uint32_t sensor_retry_start = 0;
uint32_t sensor_timeout_start = 0;

void state_sInit_enter(fsm_t *fsm) {

	rtd_horn_setup();

	setup_ctrl_steering();

	setup_pedals();

#if DRIVERLESS_CTRL == 1
	dvl_emergency_setup();
	dvl_emergency_start();

	dashASB_setup();
#endif

	if (!check_pedals_connected(&(VCU_heartbeatState.otherFlags.ctrl))) {
		// pedal sensor failed, start timers so we can retry
		sensor_retry_start = HAL_GetTick();
		sensor_timeout_start = HAL_GetTick();
	}
	else {
		// sensors good
		//setup_pedals();

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

	if ((HAL_GetTick() - sensor_retry_start) > SENSOR_RETRY) {
		if (!check_pedals_connected(&(VCU_heartbeatState.otherFlags.ctrl))) {
			// sensor failed

			sensor_retry_start = HAL_GetTick();
		}
		else {
			// sensors good, so setup filters and move to board check
			//setup_pedals();

			// disable timeout
			sensor_timeout_start = 0;

			fsm_changeState(fsm, &state_boardCheck, "Sensors initialized");
			return;
		}
	}

	if ((HAL_GetTick() - sensor_timeout_start) > SENSOR_TIMEOUT) {
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
#if DEBUG_BMU == 0
	// BMU is critical
	if (!heartbeats.BMU) {
		boards_missing = true;
	}
#endif

#if DEBUG_MCISO == 0
	// MCISO is critical
	for (int i = 0; i < MCISO_COUNT; i++) {
		if (!heartbeats.MCISO[i]) {
			boards_missing = true;
		}
	}
#endif

	if (!boards_missing) {
		// all boards required are present
		fsm_changeState(fsm, &state_checkBMU, "Boards present");
		return;
	}
}

void state_checkBMU_enter(fsm_t *fsm) {

}

void state_checkBMU_body(fsm_t *fsm) {
	// check BMU in ready state, if so go to idle

	CAN_MSG_Generic_t msg;
	while (queue_next(&queue_CAN, &msg)) {
		// check for heartbeat
		if (check_heartbeat_msg(&msg)) {
		}
		// check for shutdowns
		else if (check_shutdown_msg(&msg, &shutdown_status)) {
			if (!shutdown_status) {
				fsm_changeState(fsm, &state_shutdown, "Shutdown triggered");
				return;
			}
		}
	}

	if (!check_bad_heartbeat()) {
		// board has dropped out, go to error state
		//fsm_changeState(fsm, &state_error, "Board died");
		//return;
	}

#if DEBUG_BMU == 0
	if (BMU_hbState.stateID == BMU_STATE_READY) {
			// BMU has finished initialization so we're good to precharge whenever driver is ready

#if DRIVERLESS_CTRL == 1
		fsm_changeState(fsm, &state_DVL_EBS_check, "BMU Ready");
		return;
#else
		fsm_changeState(fsm, &state_idle, "BMU Ready");
		return;
#endif
		}
#else
	// swap instantly into ready

#if DRIVERLESS_CTRL == 1
		fsm_changeState(fsm, &state_DVL_EBS_check, "BMU Ready");
		return;
#else
		fsm_changeState(fsm, &state_idle, "BMU Ready");
		return;
#endif

#endif
}


#endif
