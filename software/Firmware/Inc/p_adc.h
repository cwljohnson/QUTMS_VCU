/*
 * p_adc.h
 *
 *  Created on: 3 Feb. 2022
 *      Author: Calvin J
 */

#ifndef INC_P_ADC_H_
#define INC_P_ADC_H_

#include <stdbool.h>
#include <Timer.h>

// 4.9v
#define ADC_DISCONNECT_CUTOFF 4900

#define ADC_CUTOFF_PULLDOWN 100

extern ms_timer_t timer_adc;

bool setup_ADC();

void ADC_timer_interrupt();

void adc_timer_cb(void *args);

#endif /* INC_P_ADC_H_ */
