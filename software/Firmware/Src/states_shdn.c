/*
 * states_shdn.c
 *
 *  Created on: 21 Feb. 2022
 *      Author: Calvin J
 */

#include "main.h"
#include "states.h"
#include "s_shutdown.h"
#include "s_suspensionTravel.h"
#include "can.h"
#include "heartbeat.h"
#include "12vSW.h"

#if VCU_CURRENT_ID == VCU_ID_SHDN

state_t state_sInit = { &state_sInit_enter, &state_sInit_body, VCU_STATE_SENSOR_INIT };
state_t state_shdn = { &state_shdn_enter, &state_shdn_body, VCU_STATE_SHDN };

uint32_t sensor_retry_start = 0;
uint32_t sensor_timeout_start = 0;

void state_sInit_enter(fsm_t *fsm) {
	setup_shutdown();

	setup_susTravel();
/*
	if (!check_susTravel_connected()) {
		// pedal sensor failed, start timers so we can retry
		sensor_retry_start = HAL_GetTick();
		sensor_timeout_start = HAL_GetTick();
	}
	else {
		// sensors good
*/
		fsm_changeState(fsm, &state_shdn, "Sensors initialized");

		sensor_retry_start = 0;
		sensor_timeout_start = 0;
		return;
/*
	}
*/
}

void state_sInit_body(fsm_t *fsm) {
	if (sensor_timeout_start == 0) {
		// everything is connected so skip this iteration
		return;
	}
	/*
	 if ((HAL_GetTick() - sensor_retry_start) > SENSOR_RETRY) {
	 if (!check_susTravel_connected()) {
	 // sensor failed

	 sensor_retry_start = HAL_GetTick();
	 }
	 else {
	 */
	// sensors good, so setup filters and move to board check
	// disable timeout
	sensor_timeout_start = 0;

	fsm_changeState(fsm, &state_shdn, "Sensors initialized");
	return;
	/*
	 }
	 }

	 if ((HAL_GetTick() - sensor_timeout_start) > SENSOR_TIMEOUT) {
	 // sensor has failed so go to error state
	 fsm_changeState(fsm, &state_error, "Sensors failed");
	 return;
	 }
	 */
}

void state_shdn_enter(fsm_t *fsm) {

	if (SW_hbState.flags._SW_Flags.FAN_ENABLE == 1) {
		// turn both fans on
		SW_setState(0, true);
		SW_setState(1, true);
	}
	else {
		// turn both fans off
		SW_setState(0, false);
		SW_setState(1, false);
	}

	return;
}

void state_shdn_body(fsm_t *fsm) {
	CAN_MSG_Generic_t msg;

	while (queue_next(&queue_CAN, &msg)) {
		// check for heartbeat
		if (check_heartbeat_msg(&msg)) {
		}
	}

	if (!check_bad_heartbeat()) {
		// something has dropped out
	}

	if (SW_hbState.flags._SW_Flags.FAN_ENABLE == 1) {
		// turn both fans on
		SW_setState(0, true);
		SW_setState(1, true);
	}
	else {
		// turn both fans off
		SW_setState(0, false);
		SW_setState(1, false);
	}


}

#endif
