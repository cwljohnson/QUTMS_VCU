/*
 * sensor_cooling.h
 *
 *  Created on: Dec 2, 2023
 *      Author: Calvin
 */

#ifndef INC_SENSOR_COOLING_H_
#define INC_SENSOR_COOLING_H_

#include "state_machine.h"


#define SW_IDX_P_SIDE_FAN (0)
#define SW_IDX_P_SIDE_PUMP (1)


void sensor_cooling_update_outputs(state_machine_t *state_machine);

#endif /* INC_SENSOR_COOLING_H_ */
