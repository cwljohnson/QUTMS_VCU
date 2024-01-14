/*
 * p_imu.h
 *
 *  Created on: 3 Feb. 2022
 *      Author: Calvin J
 */

#ifndef INC_P_IMU_H_
#define INC_P_IMU_H_

#include <stdbool.h>
#include <Timer.h>

extern ms_timer_t timer_IMU;

bool setup_IMU();
void imu_timer_cb(void *args);



#endif /* INC_P_IMU_H_ */
