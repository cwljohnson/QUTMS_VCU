/*
 * states_ctrl_driving.c
 *
 *  Created on: Feb 9, 2022
 *      Author: Calvin
 */

#include "states.h"
#include "heartbeat.h"
#include "rtd.h"
#include "shutdown.h"
#include "can.h"
#include "inverter.h"
#include "inverter_vesc.h"
#include "s_ctrl_steering.h"
#include "s_pedalBox.h"

#if VCU_CURRENT_ID == VCU_ID_CTRL

state_t state_driving = { &state_driving_enter, &state_driving_body, VCU_STATE_DRIVING };
state_t state_tsError = { &state_tsError_enter, &state_tsError_body, VCU_STATE_TS_ERROR };

ms_timer_t timer_inverters;
void inverter_timer_cb(void *args);

void state_driving_enter(fsm_t *fsm) {
	// make sure RTD light is on
	rtd_light_on();

	//inverter_setup();

	// send commands to inverters every 20ms
	timer_inverters = timer_init(30, true, inverter_timer_cb);

	timer_start(&timer_inverters);
}

void state_driving_body(fsm_t *fsm) {
	CAN_MSG_Generic_t msg;
	while (queue_next(&queue_CAN, &msg)) {
		// check for heartbeat
		if (check_heartbeat_msg(&msg)) {
		}
		// check for shutdowns
		else if (check_shutdown_msg(&msg, &shutdown_status)) {
			if (!shutdown_status) {
				inverter_shutdown();

				rtd_light_off();

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
		inverter_shutdown();

		rtd_light_off();

		fsm_changeState(fsm, &state_tsError, "Board died");
		return;
	}

	timer_update(&timer_inverters, NULL);
}

void inverter_timer_cb(void *args) {

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

	bool disable_motor = current_pedal_values.APPS_disable_motors || current_pedal_values.pedal_disable_motors;

#if BRAKE_NON_CRITICAL == 0
	disable_motor = disable_motor || current_pedal_values.BSE_disable_motors;
#endif

	inverter_send_pedals(current_pedal_values.pedal_accel_mapped[0], current_pedal_values.pedal_brake_mapped[0],
			steeringAngle, disable_motor, use_tv);
}

void state_tsError_enter(fsm_t *fsm) {
	// disable inverters
	inverter_shutdown();

	rtd_light_off();

}

void state_tsError_body(fsm_t *fsm) {

}


#endif
