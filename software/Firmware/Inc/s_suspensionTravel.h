/*
 * linearTravel_adc.h
 *
 *  Created on: Oct 12, 2021
 *      Author: Calvin
 */

#ifndef INC_LINEARTRAVEL_ADC_H_
#define INC_LINEARTRAVEL_ADC_H_

#include <Timer.h>
#include <stdbool.h>

#define ADC_CH_SUS_LEFT 3
#define ADC_CH_SUS_RIGHT 2

extern ms_timer_t timer_suspension_adc;

void setup_susTravel();

bool check_susTravel_connected();

void suspension_timer_cb(void *args);

#endif /* INC_LINEARTRAVEL_ADC_H_ */
