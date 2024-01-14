/*
 * sensor_rear_fan.h
 *
 *  Created on: Dec 9, 2023
 *      Author: Calvin
 */

#ifndef INC_SENSOR_REAR_FAN_H_
#define INC_SENSOR_REAR_FAN_H_

#include "state_machine.h"

#define SW_IDX_P_REAR_FAN (1)

void sensor_rear_fan_update_outputs(state_machine_t *state_machine);

#endif /* INC_SENSOR_REAR_FAN_H_ */
