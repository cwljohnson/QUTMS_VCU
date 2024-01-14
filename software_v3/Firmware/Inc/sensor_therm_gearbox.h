/*
 * sensor_therm_gearbox.h
 *
 *  Created on: Nov 17, 2023
 *      Author: Calvin
 */

#ifndef INC_SENSOR_THERM_GEARBOX_H_
#define INC_SENSOR_THERM_GEARBOX_H_

#include "sensor.h"

#define SENSOR_IDX_THERM_GBX (7)
#define CAN_PHASE_S_GEARBOX_TEMP (25)

void sensor_setup_therm_gearbox(void);
void sensor_update_therm_gearbox(sensor_data_t *sensor_data);
void sensor_tx_therm_gearbox(void);

#endif /* INC_SENSOR_THERM_GEARBOX_H_ */
