/*
 * s_pedalBox.h
 *
 *  Created on: Feb 9, 2022
 *      Author: Calvin
 */

#ifndef INC_S_PEDALBOX_H_
#define INC_S_PEDALBOX_H_

#include <stdint.h>
#include <stdbool.h>
#include <Timer.h>

#include <CAN_VCU.h>

#define ADC_CH_PEDAL_ACCEL_0 3
#define ADC_CH_PEDAL_ACCEL_1 5
#define ADC_CH_PEDAL_BRAKE_0 2
#define ADC_CH_PEDAL_BRAKE_1 4

#define NUM_PEDAL_ACCEL 2
#define NUM_PEDAL_BRAKE 2

#define PEDAL_ACCEL_0_MAX 2030
#define PEDAL_ACCEL_0_MIN 835
#define PEDAL_ACCEL_1_MAX 3350
#define PEDAL_ACCEL_1_MIN 2050

#define PEDAL_BRAKE_0_MAX 1200
#define PEDAL_BRAKE_0_MIN 600
#define PEDAL_BRAKE_1_MAX 1200
#define PEDAL_BRAKE_1_MIN 600

#define BRAKE_MIN_ACTUATION 500

#define PEDAL_DUTY_CYCLE 1000

#define PEDAL_IMPLAUSIBILITY_TIMEOUT 100

// is brakes missing an issue
#define BRAKE_NON_CRITICAL 0

// 10% of 1000
#define APPS_DIFF 300

typedef struct pedal_settings {
	uint16_t pedal_duty_cycle;

	uint16_t pedal_accel_min[NUM_PEDAL_ACCEL];
	uint16_t pedal_accel_max[NUM_PEDAL_ACCEL];

	uint16_t pedal_brake_min[NUM_PEDAL_BRAKE];
	uint16_t pedal_brake_max[NUM_PEDAL_BRAKE];

	uint16_t brake_min_actuation;
} pedal_settings_t;

typedef struct pedal_values {
	uint16_t pedal_accel_mapped[NUM_PEDAL_ACCEL];
	uint16_t pedal_brake_mapped[NUM_PEDAL_BRAKE];

	// To comply with T.4.2, specifically T.4.2.4 and T.4.2.5
	bool APPS_disable_motors;
	bool APPS_implausibility_present;
	uint32_t APPS_implausibility_start;

	// To comply with T.4.3.3
	bool BSE_disable_motors;
	bool BSE_implausibility_present;
	uint32_t BSE_implausibility_start;

	// To comply with EV.5.7
	bool pedal_disable_motors;
} pedal_values_t;

extern pedal_settings_t pedal_config;
extern pedal_values_t current_pedal_values;
extern ms_timer_t timer_pedal_adc;

void setup_pedals();
bool check_pedals_connected(VCU_Flags_Ctrl_u *ctrl_flags);

void pedal_timer_cb(void *args);

#endif /* INC_S_PEDALBOX_H_ */
