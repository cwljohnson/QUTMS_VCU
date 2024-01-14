/*
 * dvl_emergency.c
 *
 *  Created on: 26 Oct. 2022
 *      Author: Calvin
 */

#include "dvl_emergency.h"
#include "heartbeat.h"
#include "RTD.h"

#include <CAN_DVL.h>

ms_timer_t timer_dvl_emergency;

uint8_t dvl_counter;
uint8_t prev_dvl_hb;
bool start = false;
bool currentSirenState = false;
uint32_t dvl_siren_start;

void dvl_emergency_setup() {
	timer_dvl_emergency = timer_init(100, true, dvl_emergency_timer_cb);

}

void dvl_emergency_start() {
	timer_start(&timer_dvl_emergency);

	dvl_counter = 0;
	prev_dvl_hb = DVL_STATE_START;
	currentSirenState = false;
}

void dvl_emergency_stop() {
	timer_stop(&timer_dvl_emergency);
}

void dvl_emergency_timer_cb(void *args) {
	if (DVL_hbState.stateID == DVL_STATE_EMERGENCY) {
		if (start) {
			dvl_counter++;

			if (dvl_counter >= 5) {
				dvl_counter = 0;
				currentSirenState = !currentSirenState;
				if (currentSirenState) {
					rtd_siren_on();
				}
				else {
					rtd_siren_off();
				}
			}

			if ((HAL_GetTick() - dvl_siren_start) > 9000) {
				// running for too long
				start = false;
				rtd_siren_off();
			}
		}
		else {
			if (prev_dvl_hb != DVL_STATE_EMERGENCY) {
				// just detected transition
				start = true;
				dvl_siren_start = HAL_GetTick();
				currentSirenState = true;
				rtd_siren_on();
			}
		}
	}
	else {
		start = false;
		if (currentSirenState) {
			rtd_siren_off();
			currentSirenState = false;
		}
	}

	prev_dvl_hb = DVL_hbState.stateID;
}
