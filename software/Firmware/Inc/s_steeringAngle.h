/*
 * s_steeringAngle.h
 *
 *  Created on: 21 Feb. 2022
 *      Author: Calvin J
 */

#ifndef INC_S_STEERINGANGLE_H_
#define INC_S_STEERINGANGLE_H_

#include <Timer.h>
#include <stdbool.h>

#define ADC_CH_STEERING_0 5
#define ADC_CH_STEERING_1 4

#define NUM_STEERING 2

#define STEER_OFFSET_0 160 //175.5
#define STEER_OFFSET_1 204 //215.1

#define STEER_MIN 500
#define STEER_MAX 4500

// usually +-2 of each other, so 10 is massive bad
#define STEER_IMP_DIFF 10

typedef struct steering_settings {
	uint16_t steering_min;
	uint16_t steering_max;

	float steering_offset[NUM_STEERING];
} steering_settings_t;

typedef struct steering_values {
	double steering_mapped[NUM_STEERING];

	bool steering_disable_TV;
	bool steering_imp_present;
	uint32_t steering_imp_start;
} steering_values_t;

extern ms_timer_t timer_steering;
extern steering_settings_t steering_config;
extern steering_values_t current_steering_values;

void setup_steeringAngle();
bool check_steeringAngle_connected();

void steeringAngle_timer_cb(void *args);

#endif /* INC_S_STEERINGANGLE_H_ */
