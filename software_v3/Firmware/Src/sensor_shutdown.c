/*
 * sensor_shutdown.c
 *
 *  Created on: Oct 13, 2023
 *      Author: Calvin
 */

#include "sensor_shutdown.h"

#include "task_spi.h"
#include "can_rtos.h"

#include <CAN_VCU.h>

shutdown_status_t shutdown_status;

void sensor_setup_shutdown(void) {
	adc_mutex_acquire();

	adc_change_enabled(SENSOR_IDX_P_SHDN_0, true);
	adc_change_enabled(SENSOR_IDX_P_SHDN_1, true);
	adc_change_enabled(SENSOR_IDX_P_SHDN_2, true);
	adc_change_enabled(SENSOR_IDX_P_SHDN_3, true);
	adc_change_enabled(SENSOR_IDX_P_SHDN_STAT, true);

	adc_change_range(SENSOR_IDX_P_SHDN_0, ADS8668_RANGE_2V56);
	adc_change_range(SENSOR_IDX_P_SHDN_1, ADS8668_RANGE_2V56);
	adc_change_range(SENSOR_IDX_P_SHDN_2, ADS8668_RANGE_2V56);
	adc_change_range(SENSOR_IDX_P_SHDN_3, ADS8668_RANGE_2V56);
	adc_change_range(SENSOR_IDX_P_SHDN_STAT, ADS8668_RANGE_5V12);

	adc_mutex_release();

	shutdown_status.segs[0] = 0;
	shutdown_status.segs[1] = 0;
	shutdown_status.segs[2] = 0;
	shutdown_status.segs[3] = 0;

	shutdown_status.state = false;
}

void sensor_update_shutdown(sensor_data_t *sensor_data) {
	uint16_t shdn_reading[4];
	shdn_reading[0] = sensor_data->adc_filtered[SENSOR_IDX_P_SHDN_0];
	shdn_reading[1] = sensor_data->adc_filtered[SENSOR_IDX_P_SHDN_1];
	shdn_reading[2] = sensor_data->adc_filtered[SENSOR_IDX_P_SHDN_2];
	shdn_reading[3] = sensor_data->adc_filtered[SENSOR_IDX_P_SHDN_3];

	// > 2V -> shutdown latched is HIGH -> shutdown line is good
	uint16_t shdn_stat_raw = (sensor_data->adc_filtered[SENSOR_IDX_P_SHDN_STAT] * 1.25);
	shutdown_status.state = (shdn_stat_raw > 1500);

	for (uint8_t i = 0; i < 4; i++) {
		shutdown_status.segs[i] = ((shdn_reading[i] + 40) + 0x80) >> 8;
	}
}

void sensor_tx_shutdown(void) {
	VCU_ShutdownStatus_t segmentsMsg = Compose_VCU_ShutdownStatus(shutdown_status.segs[0], shutdown_status.segs[1], shutdown_status.segs[2],
			shutdown_status.segs[3], shutdown_status.state);

	can_msg_t msg = { 0 };
	msg.id = segmentsMsg.id;
	msg.ide = CAN_ID_EXT;
	msg.dlc = sizeof(segmentsMsg.data);
	for (uint8_t i = 0; i < msg.dlc; i++) {
		msg.data[i] = segmentsMsg.data[i];
	}

	// transmit CAN
	can_tx_enqueue(&msg, CAN_SRC_CTRL);
}
