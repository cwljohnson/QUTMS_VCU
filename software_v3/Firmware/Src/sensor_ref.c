/*
 * sensor_ref.c
 *
 *  Created on: Sep 8, 2023
 *      Author: Calvin
 */

#include "sensor_ref.h"
#include "task_spi.h"

uint16_t ref_voltage;

void sensor_setup_ref(void) {
	// these channels are on LHS of VCU, so enable current sources for pullup
	isrc_mutex_acquire();

	// configure for ~ ?mA
	isrc_change_scale(SENSOR_IDF_P_REF, MAX5548_SCALE_0);
	isrc_change_value(SENSOR_IDF_P_REF, 100);
	isrc_change_enabled(SENSOR_IDF_P_REF, true);

	isrc_mutex_release();
}

void sensor_update_ref(sensor_data_t *sensor_data) {
	ref_voltage = sensor_data->adc_filtered[SENSOR_IDF_P_REF] * ADC_SCALE_5V12;
}

uint16_t ref_get_voltage(void) {
	return ref_voltage;
}
