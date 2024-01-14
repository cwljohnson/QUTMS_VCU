/*
 * states_EBS_ADC.c
 *
 *  Created on: Dec 3, 2022
 *      Author: Calvin
 */

#include "EBS_ADC.h"
#include "states.h"
#include "heartbeat.h"
#include "can.h"

#if VCU_CURRENT_ID == VCU_ID_EBS_ADC

state_t state_sInit = { &state_sInit_enter, &state_sInit_body, VCU_STATE_SENSOR_INIT };
state_t state_ebs_adc = { &state_ebs_adc_enter, &state_ebs_adc_body, VCU_STATE_ADC_EBS };

uint32_t sensor_retry_start = 0;
uint32_t sensor_timeout_start = 0;


void state_sInit_enter(fsm_t *fsm) {
	bool success = true;

	EBS_ADC_Init();

	if (!success) {
		// sensors failed, start timers so we can retry
		sensor_retry_start = HAL_GetTick();
		sensor_timeout_start = HAL_GetTick();
	}
	else {
		// sensors good
		fsm_changeState(fsm, &state_ebs_adc, "Sensors initialized");
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

		EBS_ADC_Init();

		if (!success) {
			// sensor failed

			sensor_retry_start = HAL_GetTick();
		}
		else {
			// sensors good, so move to board check

			// disable timeout
			sensor_timeout_start = 0;

			fsm_changeState(fsm, &state_ebs_adc, "Sensors initialized");
			return;
		}
	}

	if ((sensor_timeout_start - HAL_GetTick()) > SENSOR_TIMEOUT) {
		// sensor has failed so go to error state
		fsm_changeState(fsm, &state_error, "Sensors failed");
		return;
	}
}

void state_ebs_adc_enter(fsm_t *fsm) {

}

void state_ebs_adc_body(fsm_t *fsm) {
	CAN_MSG_Generic_t msg;
	while (queue_next(&queue_CAN, &msg)) {
		// check for heartbeats
		if (check_heartbeat_msg(&msg)) {

		}
	}

	// update state of heartbeat error flags
	// don't need to go into an error state if boards aren't present
	// so won't check return value
	check_bad_heartbeat();
}

#endif
