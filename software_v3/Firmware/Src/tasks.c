/*
 * tasks.c
 *
 *  Created on: Feb 6, 2023
 *      Author: Calvin
 */

#include "tasks.h"
#include "task_spi.h"
#include "task_main.h"
#include "task_watchdog.h"

#include "inv_sevcon.h"
#include "inv_vesc.h"

void setup_tasks() {
//	setup_taskCAN();
//	setup_task_config();
//	setup_taskSensor();
	setup_taskSPI();
//	setup_task_state_machine();
	setup_task_main();
	setup_task_watchdog();

#if VCU_CURRENT_ID == VCU_ID_CTRL
#if QEV4==1
	inv_sevcon_setup();
#endif

#if QEV3==1
	inv_vesc_setup();
#endif
#endif
}

