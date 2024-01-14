/*
 * task_main.h
 *
 *  Created on: Nov 27, 2023
 *      Author: Calvin
 */

#ifndef INC_TASK_MAIN_H_
#define INC_TASK_MAIN_H_

#include "cmsis_os.h"
#include "task_spi.h"

// spi task samples ADC every 100us, want to update sensors at 100Hz
// eg 10ms -> 10000us
#define SENSOR_UPDATE_TICK (10000 / ADC_SAMPLE_US)

enum MAIN_EVT_FLAG {
	MAIN_EVT_FLAG_TIMER = 0x1,
	MAIN_EVT_FLAG_SENSOR = 0x2,
	MAIN_EVT_FLAG_CAN_RX = 0x4,
	MAIN_EVT_FLAG_CAN_TX = 0x8
};

extern osThreadId_t th_tsk_main;

void setup_task_main(void);
void main_tim_500Hz_interrupt_cb(void);

#endif /* INC_TASK_MAIN_H_ */
