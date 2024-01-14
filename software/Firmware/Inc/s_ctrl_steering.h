/*
 * s_ctrl_steering.h
 *
 *  Created on: Feb 24, 2022
 *      Author: Calvin J
 */

#ifndef INC_S_CTRL_STEERING_H_
#define INC_S_CTRL_STEERING_H_

#include <Timer.h>
#include <stdbool.h>
#include "s_steeringAngle.h"

extern steering_values_t ctrl_steering_values;
extern ms_timer_t timer_ctrl_steering;
extern bool steering_timeout;

void setup_ctrl_steering();
void ctrl_steering_timer_cb(void *args);

#endif /* INC_S_CTRL_STEERING_H_ */
