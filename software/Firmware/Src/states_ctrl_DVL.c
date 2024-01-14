/*
 * states_ctrl_DVL.c
 *
 *  Created on: Oct 17, 2022
 *      Author: Calvin J
 */

#include "states.h"
#include "can.h"

#include <CAN_VCU.h>

#include "heartbeat.h"
#include "shutdown.h"
#include "rtd.h"
#include "inverter.h"
#include "inverter_vesc.h"
#include "s_pedalBox.h"
#include "s_ctrl_steering.h"

#include <Timer.h>

#if VCU_CURRENT_ID == VCU_ID_CTRL
#if DRIVERLESS_CTRL == 1

state_t state_DVL_EBS_check = { &state_DVL_EBS_check_enter, &state_DVL_EBS_check_body, VCU_STATE_DVL_EBS_CHECK };
state_t state_DVL_RQST_mission = { &state_DVL_RQST_mission_enter, &state_DVL_RQST_mission_body,
		VCU_STATE_DVL_RQST_MISSION };
state_t state_DVL_idle = { &state_DVL_idle_enter, &state_DVL_idle_body, VCU_STATE_DVL_IDLE };
state_t state_DVL_request_pchrg = { &state_DVL_request_pchrg_enter, &state_DVL_request_pchrg_body,
		VCU_STATE_DVL_PRECHARGE_REQUEST };
state_t state_DVL_precharge = { &state_DVL_precharge_enter, &state_DVL_precharge_body, VCU_STATE_DVL_PRECHARGE };
state_t state_DVL_checkInverter = { &state_DVL_checkInverter_enter, &state_DVL_checkInverter_body,
		VCU_STATE_DVL_INVERTER_CHECK };
state_t state_DVL_rtd = { &state_DVL_rtd_enter, &state_DVL_rtd_body, VCU_STATE_DVL_RTD };
state_t state_DVL_driving = { &state_DVL_driving_enter, &state_DVL_driving_body, VCU_STATE_DVL_DRIVING };
state_t state_DVL_emergency = { &state_DVL_emergency_enter, &state_DVL_emergency_body, VCU_STATE_DVL_EMERGENCY };

ms_timer_t timer_DVL_inverters;
void DVL_inverter_timer_cb(void *args);

void state_DVL_EBS_check_enter(fsm_t *fsm) {

}

void state_DVL_EBS_check_body(fsm_t *fsm) {
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

	// update state of heartbeat error flags
	// don't need to go into an error state if boards aren't present
	// so won't check return value
	check_bad_heartbeat();

	bool EBS_present = false;

	if (heartbeats.VCU[VCU_ID_EBS]) {
		EBS_present = true;
	}

	if (EBS_present) {
		fsm_changeState(fsm, &state_DVL_RQST_mission, "EBS VCU present");
		return;
	}
}

void state_DVL_RQST_mission_enter(fsm_t *fsm) {

}

void state_DVL_RQST_mission_body(fsm_t *fsm) {
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

	// check if DVL has decided on a mission
	if (DVL_hbState.missionID == DVL_MISSION_MANUAL) {
		// manual mission selected
		fsm_changeState(fsm, &state_idle, "Manual mission selected");
		return;
	}
	else if (DVL_hbState.missionID == DVL_MISSION_SELECTED) {
		// autonomous mission selected
		fsm_changeState(fsm, &state_DVL_idle, "Autonomous mission selected");
		return;
	}
}

void state_DVL_idle_enter(fsm_t *fsm) {
	RTD_state.precharge_ticks = 0;
	rtd_setup();
}

void state_DVL_idle_body(fsm_t *fsm) {
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

		// go back to check BMU
		fsm_changeState(fsm, &state_checkBMU, "BMU not good");
		return;
	}
#endif
	if ((VCU_hbState_EBS.stateID == VCU_STATE_EBS_PCHRG_PRESSED) && (heartbeats.VCU[VCU_ID_EBS])) {
		// DVL precharge button pressed and EBS heartbeat good (eg button press valid)
		if (RTD_state.precharge_ticks == 0) {
			// first detection of press, so start counting
			RTD_state.precharge_ticks = HAL_GetTick();
		}

		if ((HAL_GetTick() - RTD_state.precharge_ticks) > DVL_PRECHARGE_BTN_TIME) {
			// precharge held for long enough
			// go into precharge

			// turn rtd btn off
			rtd_timer_off();

			// start precharge
			fsm_changeState(fsm, &state_DVL_request_pchrg, "Precharge requested");
			return;
		}
	}
	else {
		// clear counter
		RTD_state.precharge_ticks = 0;
	}
}

void state_DVL_request_pchrg_enter(fsm_t *fsm) {

}
void state_DVL_request_pchrg_body(fsm_t *fsm) {
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
		fsm_changeState(fsm, &state_DVL_precharge, "Precharging");
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

void state_DVL_precharge_enter(fsm_t *fsm) {

}
void state_DVL_precharge_body(fsm_t *fsm) {
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

		// send RTD message (this is what triggers siren)
		rtd_siren_start();

		// go to inverter health check
		fsm_changeState(fsm, &state_DVL_checkInverter, "Precharge finished");
		return;
	}
	else if ((BMU_hbState.stateID == BMU_STATE_READY) && (BMU_hbState.flags._BMU_Flags.PCHRG_TIMEOUT == 1)) {
		// precharge timed out
		// go back to idle and start again

		fsm_changeState(fsm, &state_DVL_idle, "Precharge timed out");
		return;
	}
	else if (BMU_hbState.stateID != BMU_STATE_PRECHARGE) {
		// why tf we go back, something is broken
		// go to BMU health check

		fsm_changeState(fsm, &state_checkBMU, "Precharge error");
		return;
	}
#else
		fsm_changeState(fsm, &state_DVL_checkInverter, "Precharge finished");
		return;
	#endif
}

void state_DVL_checkInverter_enter(fsm_t *fsm) {

}
void state_DVL_checkInverter_body(fsm_t *fsm) {
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
		fsm_changeState(fsm, &state_DVL_rtd, "Inverters good");
		return;
	}
}

void state_DVL_rtd_enter(fsm_t *fsm) {

}
void state_DVL_rtd_body(fsm_t *fsm) {
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

	if (DVL_hbState.stateID == DVL_STATE_DRIVING) {
		// everything good lets drive
		fsm_changeState(fsm, &state_DVL_driving, "Driving time");
		return;
	}
}

void state_DVL_driving_enter(fsm_t *fsm) {
	// send commands to inverters every 20ms
	timer_DVL_inverters = timer_init(20, true, DVL_inverter_timer_cb);

	timer_start(&timer_DVL_inverters);
}

void state_DVL_driving_body(fsm_t *fsm) {
	inverter_config.regen_enable = 1;

	CAN_MSG_Generic_t msg;
	while (queue_next(&queue_CAN, &msg)) {
		// check for heartbeat
		if (check_heartbeat_msg(&msg)) {
		}
		// check for shutdowns
		else if (check_shutdown_msg(&msg, &shutdown_status)) {
			if (!shutdown_status) {
				inverter_shutdown();

				//rtd_light_off();

				fsm_changeState(fsm, &state_shutdown, "Shutdown triggered");
				return;
			}
		}
		else if (vesc_handle_CAN(&msg)) {

		}
	}

	bool board_missing = !check_bad_heartbeat();

	bool inverter_missing = false;

	for (int i = 0; i < MCISO_COUNT; i++) {
		if ((MCISO_hbState[i].errorFlags.HB_INV0 == 1) || (MCISO_hbState[i].errorFlags.HB_INV1 == 1)) {
			inverter_missing = true;
			VCU_heartbeatState.otherFlags.ctrl._VCU_Flags_Ctrl.HB_INV = 1;
		}
	}

#if DEBUG_INV == 1
			inverter_missing = false;
	#endif

	if (board_missing || inverter_missing) {
		// board has dropped out, go to error state

		// TODO: check failure mode of this re driverless
		inverter_shutdown();

		//rtd_light_off();

		fsm_changeState(fsm, &state_tsError, "Board died");
		return;
	}

	timer_update(&timer_DVL_inverters, NULL);
}

void DVL_inverter_timer_cb(void *args) {
	// determine if we use torque vectoring
	bool use_tv = false;

	double steeringAngle = ctrl_steering_values.steering_mapped[0];

	/*
	 if (inverter_config.TV_enable == 1) {
	 if (!current_sensor_values.steering_disable_TV) {
	 use_tv = true;
	 }
	 else {
	 // we want to use TV, but it's been disable due to implausibilty of steering
	 if (!current_sensor_values.steering_imp_present) {
	 // implausibility no longer present, so if we're in steering deadzone, reenable TV

	 if (fabs(steeringAngle) < inverter_config.TV_deadzone) {
	 current_sensor_values.steering_disable_TV = false;
	 use_tv = true;
	 }
	 }
	 }
	 }

	 */

	// TODO: probably wanna disable if EBS 2 chunky
	bool disable_motor = false;
	//bool disable_motor = current_pedal_values.APPS_disable_motors || current_pedal_values.pedal_disable_motors;

#if BRAKE_NON_CRITICAL == 0
	disable_motor = disable_motor || current_pedal_values.BSE_disable_motors;
#endif

	double current_accel = DVL_hbState.torqueRequest / 100.0;

	if (!heartbeats.DVL_CTRL) {
		// DVL supervisor dropped out
		current_accel = 0;
	}

	if (RES_hbState.estop) {
		// RES has been pressed
		current_accel = 0;
	}

	uint16_t accel = 0;
	uint16_t brake = 0;

	if (current_accel > 0) {
		accel = current_accel * pedal_config.pedal_duty_cycle;
	}
	else if (current_accel < 0) {
		brake = -(current_accel * pedal_config.pedal_duty_cycle);
	}

	inverter_send_pedals(accel, brake, steeringAngle, disable_motor, use_tv);
}

void state_DVL_emergency_enter(fsm_t *fsm) {

}

void state_DVL_emergency_body(fsm_t *fsm) {
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

	// TODO: how should this be handled
}

#endif
#endif
