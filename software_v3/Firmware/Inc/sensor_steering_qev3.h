/*
 * sensor_steering_qev3.h
 *
 *  Created on: Dec 4, 2023
 *      Author: Calvin
 */

#ifndef INC_SENSOR_STEERING_QEV3_H_
#define INC_SENSOR_STEERING_QEV3_H_

#include "sensor.h"

#include <stdbool.h>
#include <stdint.h>

#define SENSOR_IDX_STEERING_0 (5)
#define SENSOR_IDX_STEERING_1 (4)

#define CAN_PHASE_S_STEERING (2)

#define NUM_STEERING (2)
#define STEERING_ADC_MAX (4990.0)

#define STEER_OFFSET_0 160 //175.5
#define STEER_OFFSET_1 204 //215.1

#define STEER_MIN 500
#define STEER_MAX 4500

// usually +-0.5 of each other, so 5 is massive bad
#define STEER_IMP_DIFF 5

typedef struct steering_settings {
	uint16_t steering_min;
	uint16_t steering_max;

	float steering_offset;
} steering_settings_t;

typedef struct steering_values {
	double steering_mapped[NUM_STEERING];
	uint16_t steering_raw[NUM_STEERING];

	double adc_max;

	bool steering_disable_TV;
	bool steering_imp_present;
	uint32_t steering_imp_start;
} steering_values_t;

void setup_sensor_steering(void);
void sensor_update_steering(sensor_data_t *sensor_data);
void sensor_steering_plausibility(void);
void sensor_tx_steering(void);

#endif /* INC_SENSOR_STEERING_QEV3_H_ */
