/*
 * sensor.c
 *
 *  Created on: Nov 27, 2023
 *      Author: Calvin
 */

#include "sensor.h"

#include "main.h"
#include "task_spi.h"

#include "sensor_ref.h"
#include "sensor_rtd.h"
#include "sensor_pedalBox.h"
#include "sensor_therm_gearbox.h"
#include "sensor_shutdown.h"
#include "sensor_suspension_rot.h"
#include "sensor_dashLED.h"
#include "sensor_steering_qev3.h"
#include "sensor_ebs_btn.h"

sensor_data_t sensor_data;

void sensor_setup(void) {
#if VCU_CURRENT_ID == VCU_ID_CTRL
	sensor_setup_pedalBox();
//	sensor_setup_ref();
	sensor_setup_rtd_btn();
#endif

#if VCU_CURRENT_ID == VCU_ID_ACCU
	sensor_setup_shutdown();
	sensor_setup_suspension_rot();
#endif

#if VCU_CURRENT_ID == VCU_ID_SENSOR
	sensor_setup_suspension_rot();
#endif

#if VCU_CURRENT_ID == VCU_ID_DASH
	sensor_setup_dashLED();
#endif

#if (VCU_CURRENT_ID == VCU_ID_COOL_L) || (VCU_CURRENT_ID == VCU_ID_COOL_R)
	sensor_setup_therm_gearbox();
#endif

#if VCU_CURRENT_ID == VCU_ID_EBS_BTN
	sensor_setup_ebs_btn();
#endif
}

void sensor_sample() {
	// sample all ADC channels
	for (uint8_t i = 0; i < ADS8668_NUM_CHANNELS; i++) {
		adc_mutex_acquire();

		sensor_data.adc_filtered[i] = adc_sample(i);
		adc_mutex_release();

	}

	// process adc channels into sensors
	sensor_update(&sensor_data);
}

void sensor_update(sensor_data_t *sensors) {
#if VCU_CURRENT_ID == VCU_ID_CTRL
//	sensor_update_ref(sensors);
	sensor_update_pedalBox(sensors);
	sensor_update_rtd_btn(sensors);
#endif

#if VCU_CURRENT_ID == VCU_ID_ACCU
	sensor_update_shutdown(sensors);
	sensor_update_suspension_rot(sensors);
#endif

#if VCU_CURRENT_ID == VCU_ID_SENSOR
	sensor_update_suspension_rot(sensors);
#endif

#if (VCU_CURRENT_ID == VCU_ID_COOL_L) || (VCU_CURRENT_ID == VCU_ID_COOL_R)
	sensor_update_therm_gearbox(sensors);
#endif

#if VCU_CURRENT_ID == VCU_ID_EBS_BTN
	sensor_update_ebs_btn(sensors);
#endif
}

void sensor_comms_tx(uint16_t phase_1hz, uint16_t phase_10hz,
		uint16_t phase_100hz) {
#if VCU_CURRENT_ID == VCU_ID_CTRL
	if (phase_100hz == CAN_PHASE_S_PEDALBOX) {
		sensor_tx_pedalBox();
	}
#endif

#if VCU_CURRENT_ID == VCU_ID_ACCU
	if (phase_100hz == CAN_PHASE_S_SHUTDOWN) {
		sensor_tx_shutdown();
	}

	if (phase_100hz == CAN_PHASE_S_SUSPENSION_ROT) {
		sensor_tx_suspension_rot();
	}

#endif

#if VCU_CURRENT_ID == VCU_ID_SENSOR
	if (phase_100hz == CAN_PHASE_S_SUSPENSION_ROT) {
		sensor_tx_suspension_rot();
	}
#endif

#if (VCU_CURRENT_ID == VCU_ID_COOL_L) || (VCU_CURRENT_ID == VCU_ID_COOL_R)
	if (phase_10hz == CAN_PHASE_S_GEARBOX_TEMP) {
		sensor_tx_therm_gearbox();
	}
#endif

#if (VCU_CURRENT_ID == VCU_ID_DASH)
	if (phase_100hz == CAN_PHASE_S_STEERING) {
		sensor_tx_steering();
	}
#endif
}
