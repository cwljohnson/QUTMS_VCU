/*
 * sensor_brakelight.h
 *
 *  Created on: Nov 6, 2023
 *      Author: Calvin
 */

#ifndef INC_SENSOR_BRAKELIGHT_H_
#define INC_SENSOR_BRAKELIGHT_H_

#include "state_machine.h"

#if QEV4 == 1
#define SW_IDX_P_BRAKELIGHT (0)
#elif QEV3 == 1
#define SW_IDX_P_BRAKELIGHT (1)
#endif

void sensor_brakelight_update_outputs(state_machine_t *state_machine);

#endif /* INC_SENSOR_BRAKELIGHT_H_ */
