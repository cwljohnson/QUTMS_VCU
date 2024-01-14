/*
 * s_steeringAngle.c
 *
 *  Created on: 21 Feb. 2022
 *      Author: Calvin J
 */

#include "s_steeringAngle.h"
#include "heartbeat.h"
#include <CAN_VCU.h>
#include "can.h"
#include "main.h"
#include "ads8668.h"
#include "p_adc.h"
#include "p_isrc.h"
#include "utilities.h"

#include <math.h>

ms_timer_t timer_steering;
steering_settings_t steering_config;
steering_values_t current_steering_values;

int steeringAngle_transmit_count = 0;

void update_steering_values();
void update_steering_plausibility();

void setup_steeringAngle() {
	// enable filters on the adcs
	ADS8668_FilterEnable(ADC_CH_STEERING_0);
	ADS8668_FilterEnable(ADC_CH_STEERING_1);

	// set range
	ADS8668_SetRange(ADC_CH_STEERING_0, ADS8668_RANGE_5V12);
	ADS8668_SetRange(ADC_CH_STEERING_1, ADS8668_RANGE_5V12);

	// set current pull ups so we can detect disconnection
	// not needed as using resistor pull downs
//	isrc_SetCurrent(ADC_CH_STEERING_0, false, 127);
//	isrc_SetCurrent(ADC_CH_STEERING_1, false, 127);

	// every 10ms check values
	timer_steering = timer_init(10, true, steeringAngle_timer_cb);

	steering_config.steering_min = STEER_MIN;
	steering_config.steering_max = STEER_MAX;

	steering_config.steering_offset[0] = STEER_OFFSET_0;
	steering_config.steering_offset[1] = STEER_OFFSET_1;

	// init count
	steeringAngle_transmit_count = 0;

	// set all plausibility checks off to start
	current_steering_values.steering_imp_present = false;
	current_steering_values.steering_disable_TV = false;

	timer_start(&timer_steering);
}

bool check_steeringAngle_connected() {
	bool success = true;

	if (ADS8668_GetScaledFiltered(ADC_CH_STEERING_0) < ADC_CUTOFF_PULLDOWN) {
		VCU_heartbeatState.otherFlags.dash._VCU_Flags_Dash.S_Steer0 = 1;
		success = false;
	}
	else {
		VCU_heartbeatState.otherFlags.dash._VCU_Flags_Dash.S_Steer0 = 0;
	}

	if (ADS8668_GetScaledFiltered(ADC_CH_STEERING_1) < ADC_CUTOFF_PULLDOWN) {
		VCU_heartbeatState.otherFlags.dash._VCU_Flags_Dash.S_Steer1 = 1;
		success = false;
	}
	else {
		VCU_heartbeatState.otherFlags.dash._VCU_Flags_Dash.S_Steer1 = 0;
	}

	return success;
}

void update_steering_values() {
	current_steering_values.steering_mapped[0] = map_capped(ADS8668_GetScaledFiltered(ADC_CH_STEERING_0),
			steering_config.steering_min, steering_config.steering_max, 0, 360);
	current_steering_values.steering_mapped[1] = map_capped(ADS8668_GetScaledFiltered(ADC_CH_STEERING_1),
				steering_config.steering_min, steering_config.steering_max, 0, 360);

	current_steering_values.steering_mapped[0] -= steering_config.steering_offset[0];
	current_steering_values.steering_mapped[1] -= steering_config.steering_offset[1];

	// negate first one to invert transfer function
	current_steering_values.steering_mapped[0] = -current_steering_values.steering_mapped[0];
}

void update_steering_plausibility() {
	current_steering_values.steering_imp_present = false;

	if (!check_steeringAngle_connected()) {
		current_steering_values.steering_imp_present = true;

	}

	double diff = fabs(current_steering_values.steering_mapped[0] - current_steering_values.steering_mapped[1]);
	if (diff > STEER_IMP_DIFF) {
		current_steering_values.steering_imp_present = true;
	}

	if(current_steering_values.steering_imp_present) {

		// current implausibility detected
		if (current_steering_values.steering_imp_start == 0) {
			current_steering_values.steering_imp_start = HAL_GetTick();
		}

		if ((HAL_GetTick() - current_steering_values.steering_imp_start) > SENSOR_IMPLAUSIBILITY_TIMEOUT) {
			// 100ms of implausibility so disable TV
			current_steering_values.steering_disable_TV = true;

			// set heartbeat flag
			VCU_heartbeatState.otherFlags.dash._VCU_Flags_Dash.IMP_Steer = 1;
		}
	} else {

		current_steering_values.steering_imp_start = 0;

		// clear heartbeat flag
		VCU_heartbeatState.otherFlags.dash._VCU_Flags_Dash.IMP_Steer = 0;
	}
}

void steeringAngle_timer_cb(void *args) {
	update_steering_values();

	update_steering_plausibility();

	// this timer runs every 20ms, want to send steering angle on CAN every 100ms, so only send every 5 calls
	steeringAngle_transmit_count++;

	if (steeringAngle_transmit_count >= 5) {
		steeringAngle_transmit_count = 0;

		// sent to CAN
		VCU_TransmitSteering_t msg = Compose_VCU_TransmitSteering(
				(int16_t)(current_steering_values.steering_mapped[0] * 10),
				(int16_t)(current_steering_values.steering_mapped[1] * 10),
				adc_readings[ADC_CH_STEERING_0], adc_readings[ADC_CH_STEERING_1]);

		CAN_TxHeaderTypeDef header = { .ExtId = msg.id, .IDE =
		CAN_ID_EXT, .RTR = CAN_RTR_DATA, .DLC = sizeof(msg.data), .TransmitGlobalTime = DISABLE, };

		send_can_msg(&hcan1, &header, msg.data);
	}
}
