/*
 * task_watchdog.c
 *
 *  Created on: Nov 27, 2023
 *      Author: Calvin
 */

#include "task_watchdog.h"

#include "task_counter.h"
#include "cmsis_os.h"
#include "iwdg.h"
#include "heartbeat.h"

osThreadId_t th_tsk_watchdog;
static uint64_t stk_tsk_watchdog[32 * 2]; // 256*2
static StaticTask_t cb_tsk_watchdog;
const osThreadAttr_t attr_tsk_watchdog = { .name = "WATCHDOG_TASK", .stack_size =
		sizeof(stk_tsk_watchdog), .stack_mem = stk_tsk_watchdog, .cb_size =
		sizeof(cb_tsk_watchdog), .cb_mem = &cb_tsk_watchdog, .priority =
		(osPriority_t) osPriorityRealtime1 };
void taskWatchdog(void *argument);

void setup_task_watchdog(void) {
	th_tsk_watchdog = osThreadNew(taskWatchdog, NULL, &attr_tsk_watchdog);

	task_counter_setup();

	// check watchdog reset state
	uint8_t wdg_reset = IWDG_CheckReset();

	// don't need mutex protection here, this is before scheduler starts
	VCU_heartbeatState.coreFlags._VCU_Flags_Core.P_Watchdog = wdg_reset;

//	// enable watchdog
	IWDG_Setup();
}

void taskWatchdog(void *argument) {
	uint32_t previous_counter_tick = osKernelGetTickCount();
	uint32_t previous_watchdog_tick = previous_counter_tick;
	uint32_t previous_state_tick = previous_counter_tick;

	bool refresh_watchdog = true;
	IWDG_Refresh();

	osDelay(150);

	IWDG_Refresh();

	for (;;) {
		uint32_t current_tick = osKernelGetTickCount();

		IWDG_Refresh();

		task_counter_mutex_acquire();

		if (!task_counter_check(&task_counters)) {
			refresh_watchdog = false;
		}
		else {
			refresh_watchdog = true;
		}

		task_counter_mutex_release();

		// wait 100ms
		osDelay(100);
	}

	// somehow exited, terminate task
	osThreadTerminate(NULL);
}
