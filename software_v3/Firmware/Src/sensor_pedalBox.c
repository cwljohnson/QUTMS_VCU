/*
 * sensor_pedalBox.c
 *
 *  Created on: Aug 5, 2023
 *      Author: Calvin
 */

#include "sensor_pedalBox.h"

#include <CAN_VCU.h>
#include <string.h>

#include "task_spi.h"
#include "can_rtos.h"
#include "util.h"
#include "heartbeat.h"

static pedal_values_t pedal_values;

pedal_settings_t pedal_settings;

void sensor_setup_pedalBox(void) {
	adc_mutex_acquire();

	adc_change_enabled(SENSOR_IDX_P_BRAKE_0, true);
	adc_change_enabled(SENSOR_IDX_P_BRAKE_1, true);
	adc_change_enabled(SENSOR_IDX_P_ACCEL_0, true);
	adc_change_enabled(SENSOR_IDX_P_ACCEL_1, true);

	adc_mutex_release();

	pedal_settings.pedal_duty_cycle = 1000.0f;
#if QEV4 == 1
	pedal_settings.pedal_offset[0] = -19.4; //-26.6;
	pedal_settings.pedal_offset[1] = -20.6; //-27.7;
	pedal_settings.pedal_travel[0] = 15.0f;
	pedal_settings.pedal_travel[1] = 15.0f;

	pedal_settings.brake_offset[0] = 380;
	pedal_settings.brake_offset[1] = 439;
#endif

#if QEV3 == 1
	pedal_settings.pedal_offset[0] = -6.9;
	pedal_settings.pedal_offset[1] = 3.6;
	pedal_settings.pedal_travel[0] = 21.0f;
	pedal_settings.pedal_travel[1] = 23.0f;

	pedal_settings.brake_offset[0] = 461;
	pedal_settings.brake_offset[1] = 515;
#endif


	pedal_settings.brake_min_actuation_psi = 30;

}

void sensor_pedalBox_APPS(void) {
	bool APPS_implausibility_check = false;

	if (pedal_values.pedal_accel_raw[0] < ADC_CUTOFF_PULLDOWN) {
		APPS_implausibility_check = true;
		VCU_heartbeatState.otherFlags.ctrl._VCU_Flags_Ctrl.S_Accel0 = 1;
	}
	else {
		VCU_heartbeatState.otherFlags.ctrl._VCU_Flags_Ctrl.S_Accel0 = 0;
	}

	if (pedal_values.pedal_accel_raw[1] < ADC_CUTOFF_PULLDOWN) {
		APPS_implausibility_check = true;
		VCU_heartbeatState.otherFlags.ctrl._VCU_Flags_Ctrl.S_Accel1 = 1;
	}
	else {
		VCU_heartbeatState.otherFlags.ctrl._VCU_Flags_Ctrl.S_Accel1 = 0;
	}

	int16_t diff = abs(
			pedal_values.pedal_accel_mapped[0]
					- pedal_values.pedal_accel_mapped[1]);
	if (diff > APPS_DIFF) {
		APPS_implausibility_check = true;
	}

	pedal_values.APPS_implausibility_present = APPS_implausibility_check;

	if (pedal_values.APPS_implausibility_present) {
		// current implausibility detected
		if (pedal_values.APPS_implausibility_start == 0) {
			pedal_values.APPS_implausibility_start = osKernelGetTickCount();
		}

		if ((osKernelGetTickCount() - pedal_values.APPS_implausibility_start)
				> PEDAL_IMPLAUSIBILITY_TIMEOUT) {
			// 100ms of implausibility, so ensure we can't send motor values
			pedal_values.APPS_disable_motors = true;

			// set heartbeat flag
			VCU_heartbeatState.otherFlags.ctrl._VCU_Flags_Ctrl.IMP_APPS = 1;
		}
	}
	else {
		pedal_values.APPS_implausibility_start = 0;
		pedal_values.APPS_disable_motors = false;

		// clear heartbeat flag
		VCU_heartbeatState.otherFlags.ctrl._VCU_Flags_Ctrl.IMP_APPS = 0;
	}
}

void sensor_pedalBox_BSE(void) {
	bool BSE_implausibility_check = false;

	if (pedal_values.pedal_brake_raw[0] < ADC_CUTOFF_PULLDOWN) {
		BSE_implausibility_check = true;
		VCU_heartbeatState.otherFlags.ctrl._VCU_Flags_Ctrl.S_Brake0 = 1;
	}
	else {
		VCU_heartbeatState.otherFlags.ctrl._VCU_Flags_Ctrl.S_Brake0 = 0;
	}

	if (pedal_values.pedal_brake_raw[1] < ADC_CUTOFF_PULLDOWN) {
		BSE_implausibility_check = true;
		VCU_heartbeatState.otherFlags.ctrl._VCU_Flags_Ctrl.S_Brake1 = 1;
	}
	else {
		VCU_heartbeatState.otherFlags.ctrl._VCU_Flags_Ctrl.S_Brake1 = 0;
	}

	// naughty, only implausibility is if sensors are disconnected
//	int diff = abs(pedal_values.pedal_brake_mapped[0] - pedal_values.pedal_brake_mapped[1]);
//	if (diff > APPS_DIFF) {
//		BSE_implausibility_check = true;
//	}

	pedal_values.BSE_implausibility_present = BSE_implausibility_check;

	if (pedal_values.BSE_implausibility_present) {
		// current implausibility detected
		if (pedal_values.BSE_implausibility_start == 0) {
			pedal_values.BSE_implausibility_start = osKernelGetTickCount();
		}

		if ((osKernelGetTickCount() - pedal_values.BSE_implausibility_start)
				> PEDAL_IMPLAUSIBILITY_TIMEOUT) {
			// 100ms of implausibility, so ensure we can't send motor values
			pedal_values.BSE_disable_motors = true;

			// set heartbeat flag
			VCU_heartbeatState.otherFlags.ctrl._VCU_Flags_Ctrl.IMP_BSE = 1;
		}
	}
	else {
		pedal_values.BSE_implausibility_start = 0;
		pedal_values.BSE_disable_motors = false;

		// clear heartbeat flag
		VCU_heartbeatState.otherFlags.ctrl._VCU_Flags_Ctrl.IMP_BSE = 0;
	}
}

void sensor_pedalBox_plausibility(void) {
	// if brake AND accel > 25% -> pedal_disable_motors = true
	if ((pedal_values.brake_pressed)
			&& (pedal_values.pedal_accel_mapped[0] > 250)) {
		pedal_values.pedal_disable_motors = true;

		// set heartbeat flag
		VCU_heartbeatState.otherFlags.ctrl._VCU_Flags_Ctrl.IMP_Pedal = 1;
	}

	// if accel < 5% -> pedal_disable_motors = false
	if (pedal_values.pedal_disable_motors) {
		if ((pedal_values.pedal_accel_mapped[0] < 50)
				|| (pedal_values.pedal_accel_mapped[1] < 50)) {
			pedal_values.pedal_disable_motors = false;

			// clear heartbeat flag
			VCU_heartbeatState.otherFlags.ctrl._VCU_Flags_Ctrl.IMP_Pedal = 0;
		}
	}
}

void sensor_update_pedalBox(sensor_data_t *sensor_data) {
	// get raw values
	uint16_t adc_p_brake_0 = sensor_data->adc_filtered[SENSOR_IDX_P_BRAKE_0]
			* ADC_SCALE_5V12;
	uint16_t adc_p_brake_1 = sensor_data->adc_filtered[SENSOR_IDX_P_BRAKE_1]
			* ADC_SCALE_5V12;
	uint16_t adc_p_accel_0 = sensor_data->adc_filtered[SENSOR_IDX_P_ACCEL_1]
			* ADC_SCALE_5V12;
	uint16_t adc_p_accel_1 = sensor_data->adc_filtered[SENSOR_IDX_P_ACCEL_0]
			* ADC_SCALE_5V12;

	pedal_values.pedal_accel_raw[0] = adc_p_accel_0;
	pedal_values.pedal_accel_raw[1] = adc_p_accel_1;

	pedal_values.pedal_brake_raw[0] = adc_p_brake_0;
	pedal_values.pedal_brake_raw[1] = adc_p_brake_1;

	pedal_values.brake_pressed = false;

	for (uint8_t i = 0; i < NUM_PEDAL_BRAKE; i++) {
		float brake_offset = ((int32_t) pedal_values.pedal_brake_raw[i])
				- pedal_settings.brake_offset[i];
//		if (brake_offset < 0) {
//			brake_offset = 0;
//		}
		pedal_values.pedal_brake_psi[i] = brake_offset * BRAKE_V_TO_PSI;
		pedal_values.pedal_brake_psi_scale[i] = pedal_values.pedal_brake_psi[i] * 16;

		pedal_values.brake_pressed = pedal_values.brake_pressed || (pedal_values.pedal_brake_psi[i] > pedal_settings.brake_min_actuation_psi);
	}

	VCU_heartbeatState.otherFlags.ctrl._VCU_Flags_Ctrl.Brake_Pressed = pedal_values.brake_pressed ? 1 : 0;

	pedal_values.pedal_accel_angle[0] = adcToAngle(
			pedal_values.pedal_accel_raw[0]);
	pedal_values.pedal_accel_angle[1] = -adcToAngle(
			pedal_values.pedal_accel_raw[1]);

	for (uint8_t i = 0; i < NUM_PEDAL_ACCEL; i++) {

		float pedal_offset = pedal_values.pedal_accel_angle[i]
				- pedal_settings.pedal_offset[i];
		if (pedal_offset < 0) {
			pedal_offset = 0;
		}

		pedal_values.pedal_accel_mapped[i] = (pedal_offset
				* pedal_settings.pedal_duty_cycle) / pedal_settings.pedal_travel[i];

		if (pedal_values.pedal_accel_mapped[i]
				> pedal_settings.pedal_duty_cycle) {
			pedal_values.pedal_accel_mapped[i] =
					pedal_settings.pedal_duty_cycle;
		}
	}

	sensor_pedalBox_APPS();
	sensor_pedalBox_BSE();
	sensor_pedalBox_plausibility();
}

void sensor_tx_pedalBox(void) {
	can_msg_t msg = { 0 };
	msg.ide = CAN_ID_EXT;

	VCU_Pedal_Accel_t accel_msg = Compose_VCU_Pedal_Accel(
			pedal_values.pedal_accel_mapped[0],
			pedal_values.pedal_accel_mapped[1], pedal_values.pedal_accel_raw[0],
			pedal_values.pedal_accel_raw[1]);

	msg.id = accel_msg.id;
	msg.dlc = sizeof(accel_msg.data);
	memcpy(msg.data, accel_msg.data, msg.dlc);

	// transmit CAN
	can_tx_enqueue(&msg, CAN_SRC_SENSOR);

	VCU_Pedal_Brake_t brake_msg = Compose_VCU_Pedal_Brake(
			pedal_values.pedal_brake_raw[0], pedal_values.pedal_brake_raw[1],
			pedal_values.pedal_brake_psi_scale[0],
			pedal_values.pedal_brake_psi_scale[1]);

	msg.id = brake_msg.id;
	msg.dlc = sizeof(brake_msg.data);
	memcpy(msg.data, brake_msg.data, msg.dlc);

	// transmit CAN
	can_tx_enqueue(&msg, CAN_SRC_SENSOR);
}

void sensor_pedalBox_getData(pedal_values_t *data) {
	*data = pedal_values;
}
