/*
 * dvl_emergency.h
 *
 *  Created on: 26 Oct. 2022
 *      Author: Calvin
 */

#ifndef INC_DVL_EMERGENCY_H_
#define INC_DVL_EMERGENCY_H_

#include <Timer.h>
#include <stdbool.h>

extern ms_timer_t timer_dvl_emergency;

void dvl_emergency_setup();

void dvl_emergency_start();
void dvl_emergency_stop();
void dvl_emergency_timer_cb(void *args);

#endif /* INC_DVL_EMERGENCY_H_ */
