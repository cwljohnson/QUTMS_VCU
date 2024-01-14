/*
 * sensor.h
 *
 *  Created on: Nov 27, 2023
 *      Author: Calvin
 */

#ifndef INC_SENSOR_H_
#define INC_SENSOR_H_

#include <stdint.h>
#include "ads8668.h"

typedef struct sensor_data {
	uint16_t adc_filtered[ADS8668_NUM_CHANNELS];
} sensor_data_t;


void sensor_setup(void);
void sensor_sample();
void sensor_update(sensor_data_t *sensors);
void sensor_comms_tx(uint16_t phase_1hz, uint16_t phase_10hz,
		uint16_t phase_100hz);

#endif /* INC_SENSOR_H_ */
