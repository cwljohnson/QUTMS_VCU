/*
 * sensor_suspension_rot.h
 *
 *  Created on: Nov 26, 2023
 *      Author: Calvin
 */

#ifndef INC_SENSOR_SUSPENSION_ROT_H_
#define INC_SENSOR_SUSPENSION_ROT_H_

#include "sensor.h"

#define SENSOR_IDX_P_SUS_L (5)
#define SENSOR_IDX_P_SUS_R (4)

#define CAN_PHASE_S_SUSPENSION_ROT (3)

void sensor_setup_suspension_rot(void);
void sensor_update_suspension_rot(sensor_data_t *sensor_data);
void sensor_tx_suspension_rot(void);

#endif /* INC_SENSOR_SUSPENSION_ROT_H_ */
