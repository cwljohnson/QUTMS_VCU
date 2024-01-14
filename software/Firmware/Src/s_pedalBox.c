/*
 * s_pedalBox.c
 *
 *  Created on: Feb 9, 2022
 *      Author: Calvin
 */

#include "s_pedalBox.h"
#include "utilities.h"

#include "ads8668.h"
#include "p_adc.h"
#include "heartbeat.h"
#include "debugCAN.h"
#include "p_isrc.h"
#include "can.h"

#include <CAN_VCU.h>

pedal_settings_t pedal_config;
pedal_values_t current_pedal_values;
ms_timer_t timer_pedal_adc;

int pedal_transmit_count = 0;

void update_sensor_values();
void update_APPS(VCU_Flags_Ctrl_u *ctrl_flags);
void update_BSE(VCU_Flags_Ctrl_u *ctrl_flags);
void update_pedal_plausibility(VCU_Flags_Ctrl_u *ctrl_flags);

void setup_pedals() {
	// enable filters on the pedal adcs
	ADS8668_FilterEnable(ADC_CH_PEDAL_ACCEL_0);
	ADS8668_FilterEnable(ADC_CH_PEDAL_ACCEL_1);
	ADS8668_FilterEnable(ADC_CH_PEDAL_BRAKE_0);
	ADS8668_FilterEnable(ADC_CH_PEDAL_BRAKE_1);

	// set range
	ADS8668_SetRange(ADC_CH_PEDAL_ACCEL_0, ADS8668_RANGE_5V12);
	ADS8668_SetRange(ADC_CH_PEDAL_ACCEL_1, ADS8668_RANGE_5V12);
	ADS8668_SetRange(ADC_CH_PEDAL_BRAKE_0, ADS8668_RANGE_5V12);
	ADS8668_SetRange(ADC_CH_PEDAL_BRAKE_1, ADS8668_RANGE_5V12);

	// pull downs for pedals

//	// ~1 mA
//	isrc_SetCurrentScale(ADC_CH_PEDAL_ACCEL_0, MAX5548_SCALE_0);
//	isrc_SetCurrentScale(ADC_CH_PEDAL_ACCEL_1, MAX5548_SCALE_0);
//	isrc_SetCurrentScale(ADC_CH_PEDAL_BRAKE_0, MAX5548_SCALE_0);
//	isrc_SetCurrentScale(ADC_CH_PEDAL_BRAKE_1, MAX5548_SCALE_0);
//
//	isrc_SetCurrentValue(ADC_CH_PEDAL_ACCEL_0, 180);
//	isrc_SetCurrentValue(ADC_CH_PEDAL_ACCEL_1, 180);
//	isrc_SetCurrentValue(ADC_CH_PEDAL_BRAKE_0, 180);
//	isrc_SetCurrentValue(ADC_CH_PEDAL_BRAKE_1, 180);
//
//	isrc_SetCurrentEnabled(ADC_CH_PEDAL_ACCEL_0, true);
//	isrc_SetCurrentEnabled(ADC_CH_PEDAL_ACCEL_1, true);
//	isrc_SetCurrentEnabled(ADC_CH_PEDAL_BRAKE_0, true);
//	isrc_SetCurrentEnabled(ADC_CH_PEDAL_BRAKE_1, true);

	// every 20ms check pedal values
	timer_pedal_adc = timer_init(20, true, pedal_timer_cb);

	// setup constants
	pedal_config.pedal_duty_cycle = PEDAL_DUTY_CYCLE;

	pedal_config.pedal_accel_min[0] = PEDAL_ACCEL_0_MIN;
	pedal_config.pedal_accel_max[0] = PEDAL_ACCEL_0_MAX;
	pedal_config.pedal_accel_min[1] = PEDAL_ACCEL_1_MIN;
	pedal_config.pedal_accel_max[1] = PEDAL_ACCEL_1_MAX;

	pedal_config.pedal_brake_min[0] = PEDAL_BRAKE_0_MIN;
	pedal_config.pedal_brake_max[0] = PEDAL_BRAKE_0_MAX;
	pedal_config.pedal_brake_min[1] = PEDAL_BRAKE_1_MIN;
	pedal_config.pedal_brake_max[1] = PEDAL_BRAKE_1_MAX;

	pedal_config.brake_min_actuation = BRAKE_MIN_ACTUATION;

	// init count
	pedal_transmit_count = 0;

	// set all plausibility checks off to start
	current_pedal_values.APPS_disable_motors = false;
	current_pedal_values.BSE_disable_motors = false;
	current_pedal_values.pedal_disable_motors = false;

	// start timer
	timer_start(&timer_pedal_adc);
}

bool check_pedals_connected(VCU_Flags_Ctrl_u *ctrl_flags) {
	bool success = true;

	if (ADS8668_GetScaledFiltered(ADC_CH_PEDAL_ACCEL_0) < ADC_CUTOFF_PULLDOWN) {
		ctrl_flags->_VCU_Flags_Ctrl.S_Accel0 = 1;

		success = false;
	}
	else {
		ctrl_flags->_VCU_Flags_Ctrl.S_Accel0 = 0;
	}

	if (ADS8668_GetScaledFiltered(ADC_CH_PEDAL_ACCEL_1) < ADC_CUTOFF_PULLDOWN) {
		ctrl_flags->_VCU_Flags_Ctrl.S_Accel1 = 1;

		success = false;
	}
	else {
		ctrl_flags->_VCU_Flags_Ctrl.S_Accel1 = 0;
	}

	if (ADS8668_GetScaledFiltered(ADC_CH_PEDAL_BRAKE_0) < ADC_CUTOFF_PULLDOWN) {
		ctrl_flags->_VCU_Flags_Ctrl.S_Brake0 = 1;
#if BRAKE_NON_CRITICAL == 0
		success = false;
#endif
	}
	else {
		ctrl_flags->_VCU_Flags_Ctrl.S_Brake0 = 0;
	}

	if (ADS8668_GetScaledFiltered(ADC_CH_PEDAL_BRAKE_1) < ADC_CUTOFF_PULLDOWN) {
		ctrl_flags->_VCU_Flags_Ctrl.S_Brake1 = 1;
#if BRAKE_NON_CRITICAL == 0
		success = false;
#endif
	}
	else {
		ctrl_flags->_VCU_Flags_Ctrl.S_Brake1 = 0;
	}

	return success;
}

void update_sensor_values() {
	// update accel
	current_pedal_values.pedal_accel_mapped[0] = map_capped(ADS8668_GetScaledFiltered(ADC_CH_PEDAL_ACCEL_0),
			pedal_config.pedal_accel_min[0], pedal_config.pedal_accel_max[0], 0, pedal_config.pedal_duty_cycle);

	current_pedal_values.pedal_accel_mapped[1] = map_capped(ADS8668_GetScaledFiltered(ADC_CH_PEDAL_ACCEL_1),
			pedal_config.pedal_accel_min[1], pedal_config.pedal_accel_max[1], 0, pedal_config.pedal_duty_cycle);

	// fix accel[0] orientation
	current_pedal_values.pedal_accel_mapped[0] = pedal_config.pedal_duty_cycle - current_pedal_values.pedal_accel_mapped[0];

	// update brakes
	current_pedal_values.pedal_brake_mapped[1] = map_capped(ADS8668_GetScaledFiltered(ADC_CH_PEDAL_BRAKE_0),
			pedal_config.pedal_brake_min[0], pedal_config.pedal_brake_max[0], 0, pedal_config.pedal_duty_cycle);

	current_pedal_values.pedal_brake_mapped[0] = map_capped(ADS8668_GetScaledFiltered(ADC_CH_PEDAL_BRAKE_1),
			pedal_config.pedal_brake_min[1], pedal_config.pedal_brake_max[1], 0, pedal_config.pedal_duty_cycle);
}

void update_APPS(VCU_Flags_Ctrl_u *ctrl_flags) {
	bool APPS_implausibility_check = false;

	if (ADS8668_GetScaledFiltered(ADC_CH_PEDAL_ACCEL_0) < ADC_CUTOFF_PULLDOWN) {
		APPS_implausibility_check = true;
		ctrl_flags->_VCU_Flags_Ctrl.S_Accel0 = 1;
	}
	else {
		ctrl_flags->_VCU_Flags_Ctrl.S_Accel0 = 0;
	}

	if (ADS8668_GetScaledFiltered(ADC_CH_PEDAL_ACCEL_1) < ADC_CUTOFF_PULLDOWN) {
		APPS_implausibility_check = true;
		ctrl_flags->_VCU_Flags_Ctrl.S_Accel1 = 1;
	}
	else {
		ctrl_flags->_VCU_Flags_Ctrl.S_Accel1 = 0;
	}

	int diff = abs(current_pedal_values.pedal_accel_mapped[0] - current_pedal_values.pedal_accel_mapped[1]);
	if (diff > APPS_DIFF) {
		APPS_implausibility_check = true;
	}

	current_pedal_values.APPS_implausibility_present = APPS_implausibility_check;

	if (current_pedal_values.APPS_implausibility_present) {
		// current implausibility detected
		if (current_pedal_values.APPS_implausibility_start == 0) {
			current_pedal_values.APPS_implausibility_start = HAL_GetTick();
		}

		if ((HAL_GetTick() - current_pedal_values.APPS_implausibility_start) > PEDAL_IMPLAUSIBILITY_TIMEOUT) {
			// 100ms of implausibility, so ensure we can't send motor values
			current_pedal_values.APPS_disable_motors = true;

			// set heartbeat flag
			ctrl_flags->_VCU_Flags_Ctrl.IMP_APPS = 1;
		}
	}
	else {
		current_pedal_values.APPS_implausibility_start = 0;
		current_pedal_values.APPS_disable_motors = false;

		// clear heartbeat flag
		ctrl_flags->_VCU_Flags_Ctrl.IMP_APPS = 0;
	}
}

void update_BSE(VCU_Flags_Ctrl_u *ctrl_flags) {
	bool BSE_implausibility_check = false;

	if (ADS8668_GetScaledFiltered(ADC_CH_PEDAL_BRAKE_0) < ADC_CUTOFF_PULLDOWN) {
		BSE_implausibility_check = true;
		ctrl_flags->_VCU_Flags_Ctrl.S_Brake0 = 1;
	}
	else {
		ctrl_flags->_VCU_Flags_Ctrl.S_Brake0 = 0;
	}

	if (ADS8668_GetScaledFiltered(ADC_CH_PEDAL_BRAKE_1) < ADC_CUTOFF_PULLDOWN) {
		BSE_implausibility_check = true;
		ctrl_flags->_VCU_Flags_Ctrl.S_Brake1 = 1;
	}
	else {
		ctrl_flags->_VCU_Flags_Ctrl.S_Brake1 = 0;
	}

	// naughty, only implausibility is if sensors are disconnected
//	int diff = abs(current_pedal_values.pedal_brake_mapped[0] - current_pedal_values.pedal_brake_mapped[1]);
//	if (diff > APPS_DIFF) {
//		BSE_implausibility_check = true;
//	}

	current_pedal_values.BSE_implausibility_present = BSE_implausibility_check;

	if (current_pedal_values.BSE_implausibility_present) {
		// current implausibility detected
		if (current_pedal_values.BSE_implausibility_start == 0) {
			current_pedal_values.BSE_implausibility_start = HAL_GetTick();
		}

		if ((HAL_GetTick() - current_pedal_values.BSE_implausibility_start) > PEDAL_IMPLAUSIBILITY_TIMEOUT) {
			// 100ms of implausibility, so ensure we can't send motor values
			current_pedal_values.BSE_disable_motors = true;

			// set heartbeat flag
			ctrl_flags->_VCU_Flags_Ctrl.IMP_BSE = 1;
		}
	}
	else {
		current_pedal_values.BSE_implausibility_start = 0;
		current_pedal_values.BSE_disable_motors = false;

		// clear heartbeat flag
		ctrl_flags->_VCU_Flags_Ctrl.IMP_BSE = 0;
	}
}

void update_pedal_plausibility(VCU_Flags_Ctrl_u *ctrl_flags) {
	// if brake AND accel > 25% -> pedal_disable_motors = true
	if ((current_pedal_values.pedal_brake_mapped[0] > pedal_config.brake_min_actuation)
			&& (current_pedal_values.pedal_accel_mapped[0] > 250)) {
		current_pedal_values.pedal_disable_motors = true;

		// set heartbeat flag
		ctrl_flags->_VCU_Flags_Ctrl.IMP_Pedal = 1;
	}

	// if accel < 5% -> pedal_disable_motors = false
	if (current_pedal_values.pedal_disable_motors) {
		if ((current_pedal_values.pedal_accel_mapped[0] < 50) || (current_pedal_values.pedal_accel_mapped[1] < 50)) {
			current_pedal_values.pedal_disable_motors = false;

			// clear heartbeat flag
			ctrl_flags->_VCU_Flags_Ctrl.IMP_Pedal = 0;
		}
	}
}

void pedal_timer_cb(void *args) {
	update_sensor_values();

	// check pedal faults
	update_APPS(&VCU_heartbeatState.otherFlags.ctrl);
	update_BSE(&VCU_heartbeatState.otherFlags.ctrl);
	update_pedal_plausibility(&VCU_heartbeatState.otherFlags.ctrl);

	// this timer runs every 20ms, want to send pedals on CAN every 100ms, so only send every 5 calls
	pedal_transmit_count++;

	if (pedal_transmit_count >= 5) {
		pedal_transmit_count = 0;

		// sent to CAN

		 VCU_Pedal_Accel_t msgAccel = Compose_VCU_Pedal_Accel(current_pedal_values.pedal_accel_mapped[0],
		 current_pedal_values.pedal_accel_mapped[1]);

		/*
		VCU_Pedal_Accel_t msgAccel = Compose_VCU_Pedal_Accel((uint16_t) ADS8668_GetScaledFiltered(ADC_CH_PEDAL_ACCEL_0),
				(uint16_t) ADS8668_GetScaledFiltered(ADC_CH_PEDAL_ACCEL_1));
				*/
		VCU_Pedal_Brake_t msgBrake = Compose_VCU_Pedal_Brake(current_pedal_values.pedal_brake_mapped[0],
				(uint16_t) ADS8668_GetScaledFiltered(ADC_CH_PEDAL_BRAKE_1),
				(uint16_t) ADS8668_GetScaledFiltered(ADC_CH_PEDAL_BRAKE_0));

		CAN_TxHeaderTypeDef header = { .ExtId = msgAccel.id, .IDE =
		CAN_ID_EXT, .RTR = CAN_RTR_DATA, .DLC = sizeof(msgAccel.data), .TransmitGlobalTime = DISABLE, };

		send_can_msg(&hcan1, &header, msgAccel.data);

		header.ExtId = msgBrake.id;
		header.DLC = sizeof(msgBrake.data);

		send_can_msg(&hcan1, &header, msgBrake.data);

		// send any error status
		if (current_pedal_values.APPS_disable_motors) {
			// send APPS error
			debugCAN_errorPresent(DEBUG_ERROR_APPS_IMPLAUSIBILITY);
		}

		if (current_pedal_values.BSE_disable_motors) {
			// send BSE error
			debugCAN_errorPresent(DEBUG_ERROR_BSE_IMPLAUSIBILITY);
		}

		if (current_pedal_values.pedal_disable_motors) {
			// send pedal plausibility error
			debugCAN_errorPresent(DEBUG_ERROR_PEDAL_IMPLAUSIBILITY);
		}
	}

}
