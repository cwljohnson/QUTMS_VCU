/*
 * states_init.c
 *
 *  Created on: Jan 18, 2022
 *      Author: Calvin
 */

#include "states.h"
#include "main.h"

#include "heartbeat.h"
#include "can_dict.h"

#include "p_watchdog.h"

#include "p_adc.h"
#include "p_imu.h"
#include "p_isrc.h"
#include "can.h"
#include "rtd.h"

state_t state_start = { &state_start_enter, &state_start_body, VCU_STATE_START };
state_t state_pInit = { &state_pInit_enter, &state_pInit_body, VCU_STATE_PERIPHERAL_INIT };
state_t state_error = { &state_error_enter, &state_error_body, VCU_STATE_ERROR };

uint32_t peripheral_retry_start = 0;
uint32_t peripheral_timeout_start = 0;

void state_start_enter(fsm_t *fsm) {
	// init object dictionary
	VCU_OD_init();

	// go to peripheral init
	fsm_changeState(fsm, &state_pInit, "Init Peripherals");

	return;
}

void state_start_body(fsm_t *fsm) {
	return;
}

void state_pInit_enter(fsm_t *fsm) {
	bool success = true;

	// setup CAN queues
	setup_CAN();

	bool CAN_good = true;

	if (!init_CAN1()) {
		VCU_heartbeatState.coreFlags._VCU_Flags_Core.P_CAN1 = 1;
		success = false;
		CAN_good = false;
	}
	else {
		// CAN has started so we can start heartbeats
		setup_heartbeat();
	}

	if (!init_CAN2()) {
		VCU_heartbeatState.coreFlags._VCU_Flags_Core.P_CAN2 = 1;
		success = false;
		CAN_good = false;
	}
	else {

	}

	if (!CAN_good) {
		VCU_heartbeatState.coreFlags._VCU_Flags_Core.P_CAN = 1;
	}

	if (!setup_IMU()) {
		VCU_heartbeatState.coreFlags._VCU_Flags_Core.P_IMU = 1;
		success = false;
	}

	// detect if reset from watchdog + start timer
	setup_watchdog();

	if (!setup_ISRC()) {
		VCU_heartbeatState.coreFlags._VCU_Flags_Core.P_ISRC = 1;
		success = false;
	}

	if (!setup_ADC()) {
		VCU_heartbeatState.coreFlags._VCU_Flags_Core.P_ADC = 1;
		success = false;
	}

	if (success) {
		fsm_changeState(fsm, &state_sInit, "Peripherals initialized");
		peripheral_retry_start = 0;
		peripheral_timeout_start = 0;
		return;
	}
	else {
		// something failed, so start timers so we can retry
		peripheral_retry_start = HAL_GetTick();
		peripheral_timeout_start = HAL_GetTick();
	}
}

void state_pInit_body(fsm_t *fsm) {
	if (peripheral_timeout_start == 0) {
		// everything is initialized so skip this iteration
		return;
	}

	// if we're here, something didn't initialize
	if ((HAL_GetTick() - peripheral_retry_start) > PERIPHERAL_RETRY) {
		uint8_t success = 0;

		if (VCU_heartbeatState.coreFlags._VCU_Flags_Core.P_CAN1 == 1) {
			success |= (1 << 0);

			// retry CAN
			if (init_CAN1()) {
				success &= ~(1 << 0);
				VCU_heartbeatState.coreFlags._VCU_Flags_Core.P_CAN1 = 0;

				// CAN has started successfully so heartbeat machine go brr
				setup_heartbeat();
			}
		}

		if (VCU_heartbeatState.coreFlags._VCU_Flags_Core.P_CAN2 == 1) {
			success |= (1 << 1);

			// retry CAN
			if (init_CAN2()) {
				success &= ~(1 << 1);
				VCU_heartbeatState.coreFlags._VCU_Flags_Core.P_CAN2 = 0;
			}
		}

		if (success == 0) {
			VCU_heartbeatState.coreFlags._VCU_Flags_Core.P_CAN = 0;
		}

		if (VCU_heartbeatState.coreFlags._VCU_Flags_Core.P_ISRC == 1) {
			success |= (1 << 2);

			// retry ADC
			if (setup_ISRC()) {
				success &= ~(1 << 2);
				VCU_heartbeatState.coreFlags._VCU_Flags_Core.P_ISRC = 0;
			}
		}

		if (VCU_heartbeatState.coreFlags._VCU_Flags_Core.P_ADC == 1) {
			success |= (1 << 3);

			// retry ADC
			if (setup_ADC()) {
				success &= ~(1 << 3);
				VCU_heartbeatState.coreFlags._VCU_Flags_Core.P_ADC = 0;
			}
		}

		if (VCU_heartbeatState.coreFlags._VCU_Flags_Core.P_IMU == 1) {
			success |= (1 << 4);

			// retry ADC
			if (setup_IMU()) {
				success &= ~(1 << 4);
				VCU_heartbeatState.coreFlags._VCU_Flags_Core.P_IMU = 0;
			}
		}

		if (success == 0) {
			// everything has initialized correctly

			// disable timeout`
			peripheral_timeout_start = 0;

			fsm_changeState(fsm, &state_sInit, "Peripherals initialized");
			return;
		}
		else {
			// something failed, so lets retry again in 100ms
			peripheral_retry_start = HAL_GetTick();
		}
	}

	if ((HAL_GetTick() - peripheral_timeout_start) > PERIPHERAL_TIMEOUT) {
		// something is clearly broken and hasn't been fixed so go to error state
		fsm_changeState(fsm, &state_error, "Peripherals failed");
		return;
	}
}

void state_error_enter(fsm_t *fsm) {
#if VCU_CURRENT_ID == VCU_ID_CTRL
	rtd_light_off();
#endif
}

void state_error_body(fsm_t *fsm) {

}
