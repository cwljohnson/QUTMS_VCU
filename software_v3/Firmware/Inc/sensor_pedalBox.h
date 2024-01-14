/*
 * sensor_pedalBox.h
 *
 *  Created on: Aug 5, 2023
 *      Author: Calvin
 */

#ifndef INC_SENSOR_PEDALBOX_H_
#define INC_SENSOR_PEDALBOX_H_

#include "sensor.h"
#include "main.h"

#include <stdint.h>
#include <stdbool.h>

#if QEV4 == 1
#define SENSOR_IDX_P_BRAKE_0 3
#define SENSOR_IDX_P_BRAKE_1 5
#define SENSOR_IDX_P_ACCEL_0 2
#define SENSOR_IDX_P_ACCEL_1 4
#endif

#if QEV3 == 1
#define SENSOR_IDX_P_BRAKE_0 2
#define SENSOR_IDX_P_BRAKE_1 4
#define SENSOR_IDX_P_ACCEL_0 5
#define SENSOR_IDX_P_ACCEL_1 3
#endif

#define CAN_PHASE_S_PEDALBOX 4

#define NUM_PEDAL_ACCEL (2)
#define NUM_PEDAL_BRAKE (2)

#define BRAKE_V_TO_PSI (2500.0/4000.0)

// 10% of 1000
#define APPS_DIFF (100)

#define BRAKE_MIN_ACTUATION (500)

// 100ms
#define PEDAL_IMPLAUSIBILITY_TIMEOUT (100)

typedef struct {
	uint16_t pedal_duty_cycle;

	float pedal_offset[NUM_PEDAL_ACCEL];
	float pedal_travel[NUM_PEDAL_ACCEL];

	uint16_t pedal_accel_min[NUM_PEDAL_ACCEL];
	uint16_t pedal_accel_max[NUM_PEDAL_ACCEL];

	uint16_t brake_offset[NUM_PEDAL_BRAKE];

	float brake_min_actuation_psi;
} pedal_settings_t;

typedef struct {
	uint16_t pedal_accel_raw[NUM_PEDAL_ACCEL];
	uint16_t pedal_accel_mapped[NUM_PEDAL_ACCEL];

	float pedal_accel_angle[NUM_PEDAL_ACCEL];

	uint16_t pedal_brake_raw[NUM_PEDAL_BRAKE];

	float pedal_brake_psi[NUM_PEDAL_BRAKE];
	int16_t pedal_brake_psi_scale[NUM_PEDAL_BRAKE];
	bool brake_pressed;

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

extern pedal_settings_t pedal_settings;

void sensor_setup_pedalBox(void);
void sensor_update_pedalBox(sensor_data_t *sensor_data);
void sensor_tx_pedalBox(void);

void sensor_pedalBox_getData(pedal_values_t *data);
void sensor_pedalBox_APPS(void);
void sensor_pedalBox_BSE(void);
void sensor_pedalBox_plausibility(void);

#endif /* INC_SENSOR_PEDALBOX_H_ */
