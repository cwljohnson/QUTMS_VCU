/*
 * p_watchdog.c
 *
 *  Created on: Mar 11, 2022
 *      Author: Calvin J
 */

#include "p_watchdog.h"
#include "heartbeat.h"
//#include "wwdg.h"
//#include "iwdg.h"
#include "main.h"

ms_timer_t timer_watchdog;

bool reset = false;

void setup_watchdog() {

	//MX_IWDG_Init();
	//MX_WWDG_Init();

	// init and start timer
	timer_watchdog = timer_init(20, true, watchdog_timer_cb);
	timer_start(&timer_watchdog);

	// check if started from watchdog reset
/*
	// check iwdg
	if (__HAL_RCC_GET_FLAG(RCC_FLAG_IWDGRST)) {
		VCU_heartbeatState.coreFlags.P_Watchdog = 1;
		reset = true;
	}

	// check wwdg
	if (__HAL_RCC_GET_FLAG(RCC_FLAG_WWDGRST)) {
		VCU_heartbeatState.coreFlags.P_Watchdog = 1;
		reset = true;
	}
*/
}

void watchdog_refresh() {
	//HAL_WWDG_Refresh(&hwwdg);
	//HAL_IWDG_Refresh(&hiwdg);
}

void watchdog_timer_cb(void *args) {
	watchdog_refresh();
}
