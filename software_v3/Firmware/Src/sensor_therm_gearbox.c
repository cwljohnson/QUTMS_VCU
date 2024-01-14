/*
 * sensor_therm_gearbox.c
 *
 *  Created on: Nov 17, 2023
 *      Author: Calvin
 */

#include "sensor_therm_gearbox.h"
#include "task_spi.h"
#include "can_rtos.h"

#include <CAN_VCU.h>
#include <math.h>

static const float LN_R0 = 9.21034;
static const float T0 = 298.15;
static const float B = 3480;

static uint16_t gearbox_adc = 0;
static uint16_t gearbox_therm_r = 0;
static float gearbox_temp = 0;

void sensor_setup_therm_gearbox(void) {
	adc_mutex_acquire();

	adc_change_enabled(SENSOR_IDX_THERM_GBX, true);
	adc_change_range(SENSOR_IDX_THERM_GBX, ADS8668_RANGE_5V12);

	adc_mutex_release();

	isrc_mutex_acquire();

	// 0.3 mA
	isrc_change_scale(SENSOR_IDX_THERM_GBX, MAX5548_SCALE_0);
	isrc_change_value(SENSOR_IDX_THERM_GBX, 64);
	isrc_change_enabled(SENSOR_IDX_THERM_GBX, true);

	isrc_mutex_release();
}

void sensor_update_therm_gearbox(sensor_data_t *sensor_data) {
	gearbox_adc = (sensor_data->adc_filtered[SENSOR_IDX_THERM_GBX] * 1.25);
	gearbox_therm_r = gearbox_adc / 0.3;

	// T = (T0 * B) / (T0 * ln(R/R0) + B)
	float num = T0 * B;
	float denum = (T0 * (logf(gearbox_therm_r) - LN_R0)) + B;
	gearbox_temp = ((num / denum) - 273.15);
}

void sensor_tx_therm_gearbox(void) {
	int16_t gearbox_temp_scale = gearbox_temp * 10;
	VCU_Temp_Gearbox_t tempMsg = Compose_VCU_Temp_Gearbox(VCU_CURRENT_ID, gearbox_adc, gearbox_therm_r, gearbox_temp_scale);

	can_msg_t msg = { 0 };
	msg.id = tempMsg.id;
	msg.ide = CAN_ID_EXT;
	msg.dlc = sizeof(tempMsg.data);
	for (uint8_t i = 0; i < msg.dlc; i++) {
		msg.data[i] = tempMsg.data[i];
	}

	// transmit CAN
	can_tx_enqueue(&msg, CAN_SRC_SENSOR);
}
