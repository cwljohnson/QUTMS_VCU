/*
 * sensor_steering_qev3.c
 *
 *  Created on: Dec 4, 2023
 *      Author: Calvin
 */

#include "sensor_steering_qev3.h"

#include <CAN_VCU.h>
#include <string.h>
#include <math.h>

#include "task_spi.h"
#include "heartbeat.h"
#include "cmsis_os2.h"

static steering_settings_t steering_config;
static steering_values_t current_steering_values;

double map_steering(double input, double adc_max) {
	// transfer function
	double output = (input * 45.0 / (0.1 * adc_max)) - 45;

	// cap output to handle out of range inputs
	if (output > 360) {
		output = 360;
	}
	else if (output < 0) {
		output = 0;
	}

	return output;
}

void setup_sensor_steering(void) {
	adc_mutex_acquire();

	adc_change_enabled(SENSOR_IDX_STEERING_0, true);
	adc_change_enabled(SENSOR_IDX_STEERING_1, true);

	adc_change_range(SENSOR_IDX_STEERING_0, ADS8668_RANGE_5V12);
	adc_change_range(SENSOR_IDX_STEERING_1, ADS8668_RANGE_5V12);

	adc_mutex_release();

	steering_config.steering_min = STEER_MIN;
	steering_config.steering_max = STEER_MAX;
	steering_config.steering_offset = STEER_OFFSET_1;

	// set all plausibility checks off to start
	current_steering_values.steering_imp_present = false;
	current_steering_values.steering_disable_TV = false;
}

void sensor_update_steering(sensor_data_t *sensor_data) {
	current_steering_values.steering_raw[0] =
			(sensor_data->adc_filtered[SENSOR_IDX_STEERING_0] * 1.25);
	current_steering_values.steering_raw[1] =
			(sensor_data->adc_filtered[SENSOR_IDX_STEERING_1] * 1.25);

	// according to math max value is sum of both outputs
	current_steering_values.adc_max = current_steering_values.steering_raw[0]
			+ current_steering_values.steering_raw[1];

	current_steering_values.steering_mapped[0] = map_steering(
			current_steering_values.steering_raw[0],
			current_steering_values.adc_max);
	current_steering_values.steering_mapped[1] = map_steering(
			current_steering_values.steering_raw[1],
			current_steering_values.adc_max);

	// flip output 0
	current_steering_values.steering_mapped[0] = 360
			- current_steering_values.steering_mapped[0];

	// apply offset to both to center values
	current_steering_values.steering_mapped[0] -=
			steering_config.steering_offset;
	current_steering_values.steering_mapped[1] -=
			steering_config.steering_offset;

	// update plausibility
	sensor_steering_plausibility();
}

void sensor_steering_plausibility(void) {
	current_steering_values.steering_imp_present = false;

	if (current_steering_values.steering_raw[0] < ADC_CUTOFF_PULLDOWN) {
		current_steering_values.steering_imp_present = true;
		VCU_heartbeatState.otherFlags.dash._VCU_Flags_Dash.S_Steer0 = 1;
	}
	else {
		VCU_heartbeatState.otherFlags.dash._VCU_Flags_Dash.S_Steer0 = 0;
	}

	if (current_steering_values.steering_raw[1] < ADC_CUTOFF_PULLDOWN) {
		current_steering_values.steering_imp_present = true;
		VCU_heartbeatState.otherFlags.dash._VCU_Flags_Dash.S_Steer1 = 1;
	}
	else {
		VCU_heartbeatState.otherFlags.dash._VCU_Flags_Dash.S_Steer1 = 0;
	}

	double diff = fabs(
			current_steering_values.steering_mapped[0]
					- current_steering_values.steering_mapped[1]);
	if (diff > STEER_IMP_DIFF) {
		current_steering_values.steering_imp_present = true;
	}

	if (current_steering_values.steering_imp_present) {
		// current implausibility detected
		if (current_steering_values.steering_imp_start == 0) {
			current_steering_values.steering_imp_start = osKernelGetTickCount();
		}

		if ((osKernelGetTickCount() - current_steering_values.steering_imp_start)
				> SENSOR_IMPLAUSIBILITY_TIMEOUT) {
			// 100ms of implausibility so disable TV
			current_steering_values.steering_disable_TV = true;

			// set heartbeat flag
			VCU_heartbeatState.otherFlags.dash._VCU_Flags_Dash.IMP_Steer = 1;
		}
	}
	else {
		current_steering_values.steering_imp_start = 0;

		// clear heartbeat flag
		VCU_heartbeatState.otherFlags.dash._VCU_Flags_Dash.IMP_Steer = 0;
	}
}

void sensor_tx_steering(void) {
	can_msg_t msg = { 0 };
	msg.ide = CAN_ID_EXT;

	VCU_TransmitSteering_t steer_msg = Compose_VCU_TransmitSteering(
			(int16_t) (current_steering_values.steering_mapped[0] * 10),
			(int16_t) (current_steering_values.steering_mapped[1] * 10),
			current_steering_values.steering_raw[0],
			current_steering_values.steering_raw[1]);

	msg.id = steer_msg.id;
	msg.dlc = sizeof(steer_msg.data);
	memcpy(msg.data, steer_msg.data, msg.dlc);

	// transmit CAN
	can_tx_enqueue(&msg, CAN_SRC_SENSOR);
}
