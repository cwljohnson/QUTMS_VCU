/*
 * MPXV7002.h
 *
 *  Created on: Oct 12, 2021
 *      Author: Calvin
 */

#ifndef INC_MPXV7002_H_
#define INC_MPXV7002_H_

#include <stdint.h>
#include <Timer.h>

float MPX_pressureTF(uint16_t voltage);

extern ms_timer_t timer_pressure;

void setup_pressure();
void pressure_timer_cb(void *args);



#endif /* INC_MPXV7002_H_ */
