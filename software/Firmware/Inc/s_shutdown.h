/*
 * s_shutdown.h
 *
 *  Created on: 21 Feb. 2022
 *      Author: Calvin J
 */

#ifndef INC_S_SHUTDOWN_H_
#define INC_S_SHUTDOWN_H_

#include <Timer.h>

#define ADC_CH_SHDN_0 1
#define ADC_CH_SHDN_1 0
#define ADC_CH_SHDN_2 6
#define ADC_CH_SHDN_3 7

#define ADC_CH_SHDN_STATUS 5

extern ms_timer_t timer_shdn_status;

void setup_shutdown();
void shdn_status_timer_cb(void *args);

#endif /* INC_S_SHUTDOWN_H_ */
