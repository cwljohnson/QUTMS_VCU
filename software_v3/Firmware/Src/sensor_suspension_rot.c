/*
 * sensor_suspension_rot.c
 *
 *  Created on: Nov 26, 2023
 *      Author: Calvin
 */

#include "sensor_suspension_rot.h"

#include <CAN_VCU.h>
#include <string.h>

#include "util.h"
#include "can_rtos.h"
#include "sensor.h"
#include "task_spi.h"

static uint16_t suspension_adc[2];
static int16_t suspension_deg[2];

void sensor_setup_suspension_rot(void) {
	adc_mutex_acquire();

	adc_change_enabled(SENSOR_IDX_P_SUS_L, true);
	adc_change_enabled(SENSOR_IDX_P_SUS_R, true);

	adc_change_range(SENSOR_IDX_P_SUS_L, ADS8668_RANGE_5V12);
	adc_change_range(SENSOR_IDX_P_SUS_R, ADS8668_RANGE_5V12);

	adc_mutex_release();
}
void sensor_update_suspension_rot(sensor_data_t *sensor_data) {
	suspension_adc[0] = sensor_data->adc_filtered[SENSOR_IDX_P_SUS_L] * 1.25;
	suspension_adc[1] = sensor_data->adc_filtered[SENSOR_IDX_P_SUS_R] * 1.25;

	suspension_deg[0] = adcToAngle(suspension_adc[0]) * 1000;
	suspension_deg[1] = adcToAngle(suspension_adc[1]) * 1000;
}

void sensor_tx_suspension_rot(void) {
	can_msg_t msg = { 0 };
	msg.ide = CAN_ID_EXT;

	VCU_Suspension_Rot_t sus_msg = Compose_VCU_Suspension_Rot(VCU_CURRENT_ID,
			suspension_adc[0], suspension_adc[1], suspension_deg[0],
			suspension_deg[1]);

	msg.id = sus_msg.id;
	msg.dlc = sizeof(sus_msg.data);
	memcpy(msg.data, sus_msg.data, msg.dlc);

	// transmit CAN
	can_tx_enqueue(&msg, CAN_SRC_SENSOR);
}
