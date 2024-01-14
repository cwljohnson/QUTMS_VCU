/*
 * states_ctrl_idle.c
 *
 *  Created on: Feb 9, 2022
 *      Author: Calvin
 */

#include "states.h"
#include "can.h"

#include <CAN_VCU.h>
#include <CAN_BMU.h>

#include "heartbeat.h"
#include "shutdown.h"
#include "rtd.h"
#include "s_pedalBox.h"

#if VCU_CURRENT_ID == VCU_ID_CTRL

state_t state_idle = { &state_idle_enter, &state_idle_body, VCU_STATE_IDLE };
state_t state_request_pchrg = { &state_request_pchrg_enter, &state_request_pchrg_body, VCU_STATE_PRECHARGE_REQUEST };
state_t state_precharge = { &state_precharge_enter, &state_precharge_body, VCU_STATE_PRECHARGE };
state_t state_checkInverter = { &state_checkInverter_enter, &state_checkInverter_body, VCU_STATE_INVERTER_CHECK };
state_t state_rtdReady = { &state_rtdReady_enter, &state_rtdReady_body, VCU_STATE_RTD_RDY };
state_t state_rtdButton = { &state_rtdButton_enter, &state_rtdButton_body, VCU_STATE_RTD_BTN };

void state_idle_enter(fsm_t *fsm) {
	rtd_setup();

	rtd_timer_on();

	RTD_state.precharge_ticks = 0;
}

void state_idle_body(fsm_t *fsm) {
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

		fsm_changeState(fsm, &state_error, "Board died");
		return;
	}
#if DEBUG_BMU == 0
	if (BMU_hbState.stateID != BMU_STATE_READY) {
		// something is wrong, BMU has had some fault, probably BMS or shutdown
		// go back to check BMU and wait for BMU to be ready again

		// turn rtd btn off
		rtd_timer_off();

		// go back to check BMU
		fsm_changeState(fsm, &state_checkBMU, "BMU not good");
		return;
	}
	else {
#endif
		if (rtd_btn_read()) {
			// precharge pressed

			if (RTD_state.precharge_ticks == 0) {
				// first detection of press, so start counting
				RTD_state.precharge_ticks = HAL_GetTick();
			}

			if ((HAL_GetTick() - RTD_state.precharge_ticks) > PRECHARGE_BTN_TIME) {
				// precharge held for long enough
				// go into precharge

				// turn rtd btn off
				rtd_timer_off();

				// start precharge
				fsm_changeState(fsm, &state_request_pchrg, "Precharge requested");
				return;
			}
		}
#if DEBUG_BMU == 0
	}
#endif

	// update timers
	timer_update(&timer_rtd, NULL);
}

void state_request_pchrg_enter(fsm_t *fsm) {

}

void state_request_pchrg_body(fsm_t *fsm) {
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

		fsm_changeState(fsm, &state_error, "Board died");
		return;
	}

#if DEBUG_BMU == 0
	if ((BMU_hbState.stateID == BMU_STATE_PRECHARGE) || (BMU_hbState.stateID == BMU_STATE_TS_ACTIVE)) {
#endif
		// precharge request has been acknowledged and started, or it's already finished so move to precharge to confirm and wait
		fsm_changeState(fsm, &state_precharge, "Precharging");
		return;
#if DEBUG_BMU == 0
	}
	else if (BMU_hbState.stateID != BMU_STATE_READY) {
		// if it's in ready, probably just about to start precharge so ignore
		// any other state is an error

		// something has clowned, so go back to BMU health check
		fsm_changeState(fsm, &state_checkBMU, "Precharge error");
		return;
	}
#endif
}

void state_precharge_enter(fsm_t *fsm) {

}

void state_precharge_body(fsm_t *fsm) {
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

		fsm_changeState(fsm, &state_error, "Board died");
		return;
	}

#if DEBUG_BMU == 0
	if (BMU_hbState.stateID == BMU_STATE_TS_ACTIVE) {
		// precharge finished successfully, TS is active
		// go to inverter health check
		fsm_changeState(fsm, &state_checkInverter, "Precharge finished");
		return;
	}
	else if ((BMU_hbState.stateID == BMU_STATE_READY) && (BMU_hbState.flags._BMU_Flags.PCHRG_TIMEOUT == 1)) {
		// precharge timed out
		// go back to idle and start again

		fsm_changeState(fsm, &state_idle, "Precharge timed out");
		return;
	}
	else if (BMU_hbState.stateID != BMU_STATE_PRECHARGE) {
		// why tf we go back, something is broken
		// go to BMU health check

		fsm_changeState(fsm, &state_checkBMU, "Precharge error");
		return;
	}
#else
	fsm_changeState(fsm, &state_checkInverter, "Precharge finished");
	return;
#endif
}

void state_checkInverter_enter(fsm_t *fsm) {

}

void state_checkInverter_body(fsm_t *fsm) {
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

		fsm_changeState(fsm, &state_error, "Board died");
		return;
	}

	bool inverter_good = true;

	for (int i = 0; i < MCISO_COUNT; i++) {
#if DEBUG_INV == 0
		// check MCISO board is good
		inverter_good = inverter_good && heartbeats.MCISO[i];

		// check connected inverters are good
		inverter_good = inverter_good && (MCISO_hbState[i].errorFlags.HB_INV0 == 0);
		inverter_good = inverter_good && (MCISO_hbState[i].errorFlags.HB_INV1 == 0);
#endif
	}

	if (inverter_good) {
		// all MCISO boards and good and all inverters report good on heartbeat so we good
		fsm_changeState(fsm, &state_rtdReady, "Inverters good");
		return;
	}
}

void state_rtdReady_enter(fsm_t *fsm) {
	// make sure button is off
	rtd_timer_off();
}

void state_rtdReady_body(fsm_t *fsm) {
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

		fsm_changeState(fsm, &state_error, "Board died");
		return;
	}

	bool brake_pressed = false;

#if (RTD_DEBUG == 1) || (BRAKE_NON_CRITICAL == 1)
	brake_pressed = true;
#else
	brake_pressed = (current_pedal_values.pedal_brake_mapped[0] >= pedal_config.brake_min_actuation);
#endif

	if (brake_pressed) {
		fsm_changeState(fsm, &state_rtdButton, "Brakes actuated");
		return;
	}
}

void state_rtdButton_enter(fsm_t *fsm) {
	// brake is actuated so turn RTD button on
	rtd_light_on();

	// brakes just got pushed, reset timer
	RTD_state.RTD_ticks = 0;
}

void state_rtdButton_body(fsm_t *fsm) {
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

		fsm_changeState(fsm, &state_error, "Board died");
		return;
	}

	bool brake_pressed = false;

#if (RTD_DEBUG == 1) || (BRAKE_NON_CRITICAL == 1)
	brake_pressed = true;
#else
	brake_pressed = (current_pedal_values.pedal_brake_mapped[0] >=  pedal_config.brake_min_actuation);
#endif

	if (!brake_pressed) {
		fsm_changeState(fsm, &state_rtdReady, "Brakes not actuated");
		return;
	}

	if (rtd_btn_read()) {
		if (RTD_state.RTD_ticks == 0) {
			// button just pushed
			RTD_state.RTD_ticks = HAL_GetTick();
		}

		uint32_t timeLeft = (HAL_GetTick() - RTD_state.RTD_ticks);

		//printf("total: %i, start: %i\r\n", timeLeft, RTD_state.RTD_ticks);

		if (timeLeft > RTD_BTN_TIME) {
			// send RTD message (this is what triggers siren)
			rtd_siren_start();

			// broadcast RTD over CAN
			rtd_broadcast();

			// change to driving state
			fsm_changeState(fsm, &state_driving, "RTD Pressed");
			return;
		}
	}
	else {
		// button not pressed, reset timer
		RTD_state.RTD_ticks = 0;
	}
}

#endif
