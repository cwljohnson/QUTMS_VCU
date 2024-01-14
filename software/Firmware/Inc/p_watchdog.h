/*
 * p_watchdog.h
 *
 *  Created on: Mar 11, 2022
 *      Author: Calvin J
 */

#ifndef INC_P_WATCHDOG_H_
#define INC_P_WATCHDOG_H_

#include <Timer.h>

extern ms_timer_t timer_watchdog;

void setup_watchdog();
void watchdog_refresh();
void watchdog_timer_cb(void *args);

#endif /* INC_P_WATCHDOG_H_ */
