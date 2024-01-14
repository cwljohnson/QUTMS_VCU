/*
 * sensor_ref.h
 *
 *  Created on: Sep 8, 2023
 *      Author: Calvin
 */

#ifndef INC_SENSOR_REF_H_
#define INC_SENSOR_REF_H_

#include "sensor.h"

#define SENSOR_IDF_P_REF 7

void sensor_setup_ref(void);
void sensor_update_ref(sensor_data_t *sensor_data);

uint16_t ref_get_voltage(void);

#endif /* INC_SENSOR_REF_H_ */
