/*
 * states_EBS.c
 *
 *  Created on: 7 Jul. 2022
 *      Author: Alex Pearl
 */

#include "states.h"

#include <QUTMS_CAN.h>
#include <CAN_VCU.h>
#include <CAN_DVL.h>

#include "shutdown.h"
#include "can.h"
#include "EBS.h"
#include "heartbeat.h"

#if VCU_CURRENT_ID == VCU_ID_EBS

state_t state_sInit = { &state_sInit_enter, &state_sInit_body, VCU_STATE_SENSOR_INIT };
state_t state_boardCheck = { &state_boardCheck_enter, &state_boardCheck_body, VCU_STATE_BOARD_CHECK };

state_t state_ebs_pwr = { &state_ebs_pwr_enter, &state_ebs_pwr_body, VCU_STATE_EBS_PWR };
state_t state_ebs_check_asms = { &state_ebs_check_asms_enter, &state_ebs_check_asms_body, VCU_STATE_EBS_CHECK_ASMS };
state_t state_ebs_check_pressure = { &state_ebs_check_pressure_enter, &state_ebs_check_pressure_body,
		VCU_STATE_EBS_CHECK_PRESSURE };

state_t state_ebs_check_pressure_btn = { &state_ebs_check_pressure_btn_enter, &state_ebs_check_pressure_btn_body,
		VCU_STATE_EBS_CHECK_PRESSURE_BTN };
state_t state_ebs_check_pressure_low = { &state_ebs_check_pressure_low_enter, &state_ebs_check_pressure_low_body,
		VCU_STATE_EBS_CHECK_PRESSURE_LOW };
state_t state_ebs_check_pressure_high = { &state_ebs_check_pressure_high_enter, &state_ebs_check_pressure_high_body,
		VCU_STATE_EBS_CHECK_PRESSURE_HIGH };

state_t state_ebs_ctrl_ack = { &state_ebs_ctrl_ack_enter, &state_ebs_ctrl_ack_body, VCU_STATE_EBS_CTRL_ACK };
state_t state_ebs_idle = { &state_ebs_idle_enter, &state_ebs_idle_body, VCU_STATE_EBS_IDLE };
state_t state_ebs_pchrg_pressed = { &state_ebs_pchrg_pressed_enter, &state_ebs_pchrg_pressed_body,
		VCU_STATE_EBS_PCHRG_PRESSED };

state_t state_ebs_check_ts = { &state_ebs_check_ts_enter, &state_ebs_check_ts_body, VCU_STATE_EBS_CHECK_TS };
state_t state_ebs_check_compute = { &state_ebs_check_compute_enter, &state_ebs_check_compute_body,
		VCU_STATE_EBS_CHECK_COMPUTE };
state_t state_ebs_ready = { &state_ebs_ready_enter, &state_ebs_ready_body, VCU_STATE_EBS_READY };
state_t state_ebs_release_brake = { &state_ebs_release_brake_enter, &state_ebs_release_brake_body, VCU_STATE_EBS_RELEASE_BRAKE };
state_t state_ebs_drive = { &state_ebs_drive_enter, &state_ebs_drive_body, VCU_STATE_EBS_DRIVE };
state_t state_ebs_braking = { &state_ebs_braking_enter, &state_ebs_braking_body, VCU_STATE_EBS_BRAKING };

uint32_t sensor_retry_start = 0;
uint32_t sensor_timeout_start = 0;

uint16_t brake_adc_rear = 0;
uint16_t brake_adc_front = 0;

void state_sInit_enter(fsm_t *fsm) {
	bool success = true;

	EBS_Init();

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

		EBS_Init();

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
	CAN_MSG_Generic_t msg;
	while (queue_next(&queue_CAN, &msg)) {
		// check for heartbeats
		if (check_heartbeat_msg(&msg)) {

		}
		else if (msg.ID == VCU_Pedal_Brake_ID) {
			uint16_t dummy;
			Parse_VCU_Pedal_Brake(msg.data, &dummy, &brake_adc_front, &brake_adc_rear);
		}
	}

	// update state of heartbeat error flags
	// don't need to go into an error state if boards aren't present
	// so won't check return value
	check_bad_heartbeat();

	bool boards_missing = false;

	if (!boards_missing) {
		// all boards required are present

		fsm_changeState(fsm, &state_ebs_pwr, "Boards present");
		return;
	}
}

void state_ebs_pwr_enter(fsm_t *fsm) {
	/*
	 // enable RES lol
	 uint8_t data[2] = { 0x01, 0x00 };

	 CAN_TxHeaderTypeDef	header = {.StdId = 0x00, .IDE = CAN_ID_STD, .RTR = CAN_RTR_DATA, .DLC = 2, .TransmitGlobalTime = DISABLE};

	 // send heartbeat
	 send_can_msg(&hcan1, &header, data);
	 send_can_msg(&hcan2, &header, data);
	 */

	//ADS8668_Background_Disable();
	// setup control lines
	EBS_SetCtrlEBS(false);
	EBS_SetCtrlSHDN(true);
	//ADS8668_Background_Enable();

	HAL_Delay(1);

	// turn on EBS board
	EBS_SetPWR(true);

	// go to check ASMS
	fsm_changeState(fsm, &state_ebs_check_asms, "EBS On");
	return;
}

void state_ebs_pwr_body(fsm_t *fsm) {
	// turn on EBS board
	EBS_SetPWR(true);
}

void state_ebs_check_asms_enter(fsm_t *fsm) {

}

void state_ebs_check_asms_body(fsm_t *fsm) {
	CAN_MSG_Generic_t msg;
	while (queue_next(&queue_CAN, &msg)) {
		// check for heartbeats
		if (check_heartbeat_msg(&msg)) {

		}
		else if (msg.ID == VCU_Pedal_Brake_ID) {
			uint16_t dummy;
			Parse_VCU_Pedal_Brake(msg.data, &dummy, &brake_adc_front, &brake_adc_rear);
		}
		else if (check_shutdown_msg(&msg, &shutdown_status)) {
			if (!shutdown_status) {

				fsm_changeState(fsm, &state_shutdown, "Shutdown triggered");
				return;
			}
		}
	}

	check_bad_heartbeat();

#if DEBUG_EBS_ASMS_CHECK == 0
	if (EBS_GetDet24V()) {
#endif
		fsm_changeState(fsm, &state_ebs_check_pressure, "ASMS Good");

#if DEBUG_EBS_ASMS_CHECK == 0
	}
#endif
}

void state_ebs_check_pressure_enter(fsm_t *fsm) {

}

void state_ebs_check_pressure_body(fsm_t *fsm) {
	CAN_MSG_Generic_t msg;
	while (queue_next(&queue_CAN, &msg)) {
		// check for heartbeats
		if (check_heartbeat_msg(&msg)) {

		}
		else if (msg.ID == VCU_Pedal_Brake_ID) {
			uint16_t dummy;
			Parse_VCU_Pedal_Brake(msg.data, &dummy, &brake_adc_front, &brake_adc_rear);
		}
		else if (check_shutdown_msg(&msg, &shutdown_status)) {
			if (!shutdown_status) {

				fsm_changeState(fsm, &state_shutdown, "Shutdown triggered");
				return;
			}
		}
	}

	check_bad_heartbeat();
#if DEBUG_EBS_ASMS_CHECK == 0
	if (!EBS_GetDet24V()) {
		fsm_changeState(fsm, &state_ebs_pwr, "ASMS bad");
	}
#endif

	if ((brake_adc_front > EBS_PRESSURE_THRESH_HIGH)) {	//&& (brake_adc_rear > EBS_PRESSURE_THRESH_HIGH)) {
		// both rear and front should be braking at this point

#if DEBUG_EBS_BRAKE_CHECK == 0
		fsm_changeState(fsm, &state_ebs_check_pressure_btn, "Brake pressure high");
		return;
#else
		fsm_changeState(fsm, &state_ebs_ctrl_ack, "Brake pressure high");
		return;
#endif
	}
}

uint32_t btn_tgl_timer_start = 0;
uint32_t btn_timer_start = 0;
bool btn_state = false;

void state_ebs_check_pressure_btn_enter(fsm_t *fsm) {
	btn_timer_start = 0;
	btn_tgl_timer_start = HAL_GetTick();
	btn_state = true;
	EBS_SetPChrgBtnLED(btn_state);
}

void state_ebs_check_pressure_btn_body(fsm_t *fsm) {
	CAN_MSG_Generic_t msg;
	while (queue_next(&queue_CAN, &msg)) {
		// check for heartbeats
		if (check_heartbeat_msg(&msg)) {

		}
		else if (msg.ID == VCU_Pedal_Brake_ID) {
			uint16_t dummy;
			Parse_VCU_Pedal_Brake(msg.data, &dummy, &brake_adc_front, &brake_adc_rear);
		}
		else if (check_shutdown_msg(&msg, &shutdown_status)) {
			if (!shutdown_status) {

				btn_state = false;
				EBS_SetPChrgBtnLED(btn_state);
				fsm_changeState(fsm, &state_shutdown, "Shutdown triggered");
				return;
			}
		}
	}

	if ((HAL_GetTick() - btn_tgl_timer_start) > EBS_BTN_TOGGLE) {
		// toggle every 500ms
		btn_state = !btn_state;
		EBS_SetPChrgBtnLED(btn_state);
		btn_tgl_timer_start = HAL_GetTick();
	}

	if (EBS_GetPChrgBtn()) {
		if (btn_timer_start == 0) {
			btn_timer_start = HAL_GetTick();
		}
		else {
			if ((HAL_GetTick() - btn_timer_start) > EBS_BTN_HOLD_TIME) {
				btn_state = false;
				EBS_SetPChrgBtnLED(btn_state);
				fsm_changeState(fsm, &state_ebs_check_pressure_low, "Pressure test activated");
			}
		}
	}
	else {
		btn_timer_start = 0;
	}
}

uint32_t pressure_low_start;

void state_ebs_check_pressure_low_enter(fsm_t *fsm) {
	// activate EBS
	EBS_SetCtrlEBS(true);

	pressure_low_start = HAL_GetTick();
}

void state_ebs_check_pressure_low_body(fsm_t *fsm) {
	CAN_MSG_Generic_t msg;
	while (queue_next(&queue_CAN, &msg)) {
		// check for heartbeats
		if (check_heartbeat_msg(&msg)) {

		}
		else if (msg.ID == VCU_Pedal_Brake_ID) {
			uint16_t dummy;
			Parse_VCU_Pedal_Brake(msg.data, &dummy, &brake_adc_front, &brake_adc_rear);
		}
		else if (check_shutdown_msg(&msg, &shutdown_status)) {
			if (!shutdown_status) {

				fsm_changeState(fsm, &state_shutdown, "Shutdown triggered");
				return;
			}
		}
	}

	// check brake pressure is low now EBS is activated
	if ((brake_adc_front < EBS_PRESSURE_THRESH_LOW) && (brake_adc_rear < EBS_PRESSURE_THRESH_LOW)) {

		if (pressure_low_start == 0) {
			pressure_low_start = HAL_GetTick();
		}
		else {
			if ((HAL_GetTick() - pressure_low_start) > 2000) {
				// both rear and front should be braking at this point
				fsm_changeState(fsm, &state_ebs_check_pressure_high, "Brake pressure low");
				return;
			}
		}
	}
	else {
		pressure_low_start = 0;
	}
}

void state_ebs_check_pressure_high_enter(fsm_t *fsm) {
	// deactivate EBS
	EBS_SetCtrlEBS(false);
}

void state_ebs_check_pressure_high_body(fsm_t *fsm) {
	CAN_MSG_Generic_t msg;
	while (queue_next(&queue_CAN, &msg)) {
		// check for heartbeats
		if (check_heartbeat_msg(&msg)) {

		}
		else if (msg.ID == VCU_Pedal_Brake_ID) {
			uint16_t dummy;
			Parse_VCU_Pedal_Brake(msg.data, &dummy, &brake_adc_front, &brake_adc_rear);
		}
		else if (check_shutdown_msg(&msg, &shutdown_status)) {
			if (!shutdown_status) {

				fsm_changeState(fsm, &state_shutdown, "Shutdown triggered");
				return;
			}
		}
	}

	// check brake pressure is high again now EBS is deactivated
	if ((brake_adc_front > EBS_PRESSURE_THRESH_HIGH) && (brake_adc_rear > EBS_PRESSURE_THRESH_HIGH)) {
		fsm_changeState(fsm, &state_ebs_ctrl_ack, "Brake pressure high");
		return;
	}
}

void state_ebs_ctrl_ack_enter(fsm_t *fsm) {

}

void state_ebs_ctrl_ack_body(fsm_t *fsm) {
	CAN_MSG_Generic_t msg;
	while (queue_next(&queue_CAN, &msg)) {
		// check for heartbeats
		if (check_heartbeat_msg(&msg)) {

		}
		else if (msg.ID == VCU_Pedal_Brake_ID) {
			uint16_t dummy;
			Parse_VCU_Pedal_Brake(msg.data, &dummy, &brake_adc_front, &brake_adc_rear);
		}
		else if (check_shutdown_msg(&msg, &shutdown_status)) {
			if (!shutdown_status) {

				fsm_changeState(fsm, &state_shutdown, "Shutdown triggered");
				return;
			}
		}
	}

	check_bad_heartbeat();

#if DEBUG_EBS_ASMS_CHECK == 0
	if (!EBS_GetDet24V()) {
		fsm_changeState(fsm, &state_ebs_pwr, "ASMS bad");
	}
#endif

	if (VCU_hbState_CTRL.stateID == VCU_STATE_DVL_IDLE) {
		// CTRL VCU in driverless IDLE -> we have a mission and can continue
		fsm_changeState(fsm, &state_ebs_idle, "CTRL ready");
		return;
	}
}

void state_ebs_idle_enter(fsm_t *fsm) {

}

void state_ebs_idle_body(fsm_t *fsm) {
	CAN_MSG_Generic_t msg;
	while (queue_next(&queue_CAN, &msg)) {
		// check for heartbeats
		if (check_heartbeat_msg(&msg)) {

		}
		else if (msg.ID == VCU_Pedal_Brake_ID) {
			uint16_t dummy;
			Parse_VCU_Pedal_Brake(msg.data, &dummy, &brake_adc_front, &brake_adc_rear);
		}
		else if (check_shutdown_msg(&msg, &shutdown_status)) {
			if (!shutdown_status) {
				EBS_SetPChrgBtnLED(false);
				fsm_changeState(fsm, &state_shutdown, "Shutdown triggered");
				return;
			}
		}
	}

	check_bad_heartbeat();

	EBS_SetPChrgBtnLED(true);

#if DEBUG_EBS_ASMS_CHECK == 0
	if (!EBS_GetDet24V()) {
		fsm_changeState(fsm, &state_ebs_pwr, "ASMS bad");
	}
#endif

	if (EBS_GetPChrgBtn()) {
		// button pressed
		fsm_changeState(fsm, &state_ebs_pchrg_pressed, "Button pressed");
		return;
	}
}

void state_ebs_pchrg_pressed_enter(fsm_t *fsm) {

}

void state_ebs_pchrg_pressed_body(fsm_t *fsm) {
	CAN_MSG_Generic_t msg;
	while (queue_next(&queue_CAN, &msg)) {
		// check for heartbeats
		if (check_heartbeat_msg(&msg)) {

		}
		else if (msg.ID == VCU_Pedal_Brake_ID) {
			uint16_t dummy;
			Parse_VCU_Pedal_Brake(msg.data, &dummy, &brake_adc_front, &brake_adc_rear);
		}
		else if (check_shutdown_msg(&msg, &shutdown_status)) {
			if (!shutdown_status) {

				fsm_changeState(fsm, &state_shutdown, "Shutdown triggered");
				return;
			}
		}
	}

	check_bad_heartbeat();

	EBS_SetPChrgBtnLED(false);

#if DEBUG_EBS_ASMS_CHECK == 0
	if (!EBS_GetDet24V()) {
		fsm_changeState(fsm, &state_ebs_pwr, "ASMS bad");
	}
#endif

	if (!EBS_GetPChrgBtn()) {
		// button pressed
		fsm_changeState(fsm, &state_ebs_idle, "Button not pressed");
		return;
	}

	if (VCU_hbState_CTRL.stateID == VCU_STATE_DVL_PRECHARGE) {
		// CTRL VCU in driverless IDLE -> we have a mission and can continue
		fsm_changeState(fsm, &state_ebs_check_ts, "Precharging");
		return;
	}
}

void state_ebs_check_ts_enter(fsm_t *fsm) {

}

void state_ebs_check_ts_body(fsm_t *fsm) {
	CAN_MSG_Generic_t msg;
	while (queue_next(&queue_CAN, &msg)) {
		// check for heartbeats
		if (check_heartbeat_msg(&msg)) {

		}
		else if (msg.ID == VCU_Pedal_Brake_ID) {
			uint16_t dummy;
			Parse_VCU_Pedal_Brake(msg.data, &dummy, &brake_adc_front, &brake_adc_rear);
		}
		else if (check_shutdown_msg(&msg, &shutdown_status)) {
			if (!shutdown_status) {

				fsm_changeState(fsm, &state_shutdown, "Shutdown triggered");
				return;
			}
		}
	}

	check_bad_heartbeat();

#if DEBUG_EBS_ASMS_CHECK == 0
	if (!EBS_GetDet24V()) {
		fsm_changeState(fsm, &state_ebs_pwr, "ASMS bad");
	}
#endif

	if (VCU_hbState_CTRL.stateID == VCU_STATE_DVL_IDLE) {
		// precharge failed
		fsm_changeState(fsm, &state_ebs_idle, "Precharge failed");
		return;
	}

	if (VCU_hbState_CTRL.stateID == VCU_STATE_DVL_RTD) {
		// CTRL VCU in driverless IDLE -> we have a mission and can continue
		fsm_changeState(fsm, &state_ebs_check_compute, "Precharge finished");
		return;
	}
}

void state_ebs_check_compute_enter(fsm_t *fsm) {

}

void state_ebs_check_compute_body(fsm_t *fsm) {
	CAN_MSG_Generic_t msg;
	while (queue_next(&queue_CAN, &msg)) {
		// check for heartbeats
		if (check_heartbeat_msg(&msg)) {

		}
		else if (msg.ID == VCU_Pedal_Brake_ID) {
			uint16_t dummy;
			Parse_VCU_Pedal_Brake(msg.data, &dummy, &brake_adc_front, &brake_adc_rear);
		}
		else if (check_shutdown_msg(&msg, &shutdown_status)) {
			if (!shutdown_status) {

				fsm_changeState(fsm, &state_shutdown, "Shutdown triggered");
				return;
			}
		}
	}

	check_bad_heartbeat();

#if DEBUG_EBS_ASMS_CHECK == 0
	if (!EBS_GetDet24V()) {
		fsm_changeState(fsm, &state_ebs_pwr, "ASMS bad");
	}
#endif

	if (DVL_hbState.stateID == DVL_STATE_CHECK_EBS) {
		// DVL CTRL is vibing
		fsm_changeState(fsm, &state_ebs_ready, "DVL waiting for EBS");
		return;
	}
}

void state_ebs_ready_enter(fsm_t *fsm) {

}

void state_ebs_ready_body(fsm_t *fsm) {
	CAN_MSG_Generic_t msg;
	while (queue_next(&queue_CAN, &msg)) {
		// check for heartbeats
		if (check_heartbeat_msg(&msg)) {

		}
		else if (msg.ID == VCU_Pedal_Brake_ID) {
			uint16_t dummy;
			Parse_VCU_Pedal_Brake(msg.data, &dummy, &brake_adc_front, &brake_adc_rear);
		}
		else if (check_shutdown_msg(&msg, &shutdown_status)) {
			if (!shutdown_status) {

				fsm_changeState(fsm, &state_shutdown, "Shutdown triggered");
				return;
			}
		}
	}

	check_bad_heartbeat();

#if DEBUG_EBS_ASMS_CHECK == 0
	if (!EBS_GetDet24V()) {
		fsm_changeState(fsm, &state_ebs_pwr, "ASMS bad");
	}
#endif

	if (DVL_hbState.stateID == DVL_STATE_RELEASE_EBS) {
		// DVL CTRL is vibing
		fsm_changeState(fsm, &state_ebs_release_brake, "Releasing brake");
		return;
	}
}

void state_ebs_release_brake_enter(fsm_t *fsm) {
	// driving mode, so activate EBS
	EBS_SetCtrlEBS(true);
}

void state_ebs_release_brake_body(fsm_t *fsm) {
	CAN_MSG_Generic_t msg;
	while (queue_next(&queue_CAN, &msg)) {
		// check for heartbeats
		if (check_heartbeat_msg(&msg)) {

		}
		else if (msg.ID == VCU_Pedal_Brake_ID) {
			uint16_t dummy;
			Parse_VCU_Pedal_Brake(msg.data, &dummy, &brake_adc_front, &brake_adc_rear);
		}
		else if (check_shutdown_msg(&msg, &shutdown_status)) {
			if (!shutdown_status) {

				fsm_changeState(fsm, &state_shutdown, "Shutdown triggered");
				return;
			}
		}
	}

	check_bad_heartbeat();

#if DEBUG_EBS_ASMS_CHECK == 0
	if (!EBS_GetDet24V()) {
		fsm_changeState(fsm, &state_ebs_pwr, "ASMS bad");
	}
#endif

	// if brake pressure has dropped, go to driving
	if ((brake_adc_front < EBS_PRESSURE_THRESH_LOW)) {
		fsm_changeState(fsm, &state_ebs_drive, "DVL in driving");
		return;
	}
}

void state_ebs_drive_enter(fsm_t *fsm) {
}

void state_ebs_drive_body(fsm_t *fsm) {
	CAN_MSG_Generic_t msg;
	while (queue_next(&queue_CAN, &msg)) {
		// check for heartbeats
		if (check_heartbeat_msg(&msg)) {

		}
		else if (msg.ID == VCU_Pedal_Brake_ID) {
			uint16_t dummy;
			Parse_VCU_Pedal_Brake(msg.data, &dummy, &brake_adc_front, &brake_adc_rear);
		}
		else if (check_shutdown_msg(&msg, &shutdown_status)) {
			if (!shutdown_status) {

				fsm_changeState(fsm, &state_shutdown, "Shutdown triggered");
				return;
			}
		}
	}

	check_bad_heartbeat();

#if DEBUG_EBS_ASMS_CHECK == 0
	if (!EBS_GetDet24V()) {
		fsm_changeState(fsm, &state_ebs_pwr, "ASMS bad");
	}
#endif

	if (DVL_hbState.stateID == DVL_STATE_ACTIVATE_EBS) {
		// DVL CTRL requested EBS braking
		fsm_changeState(fsm, &state_ebs_braking, "Brake time");
		return;
	}
}

void state_ebs_braking_enter(fsm_t *fsm) {
	// deactivate EBS
	EBS_SetCtrlEBS(false);

	// trigger shutdown
	//EBS_SetCtrlSHDN(false);
}

void state_ebs_braking_body(fsm_t *fsm) {
	CAN_MSG_Generic_t msg;
	while (queue_next(&queue_CAN, &msg)) {
		// check for heartbeats
		if (check_heartbeat_msg(&msg)) {

		}
		else if (msg.ID == VCU_Pedal_Brake_ID) {
			uint16_t dummy;
			Parse_VCU_Pedal_Brake(msg.data, &dummy, &brake_adc_front, &brake_adc_rear);
		}
		else if (check_shutdown_msg(&msg, &shutdown_status)) {
			// ignore shutdown status if manually activated EBS -> don't wanna restore

			// TODO: this can come back in when the EBS ADC stops dying lol

			/*
			 if (!shutdown_status) {

			 fsm_changeState(fsm, &state_shutdown, "Shutdown triggered");
			 return;
			 }
			 */
		}
	}

	check_bad_heartbeat();

#if DEBUG_EBS_ASMS_CHECK == 0
	if (!EBS_GetDet24V()) {
		fsm_changeState(fsm, &state_ebs_pwr, "ASMS bad");
	}
#endif
}

#endif
