/*
 * task_main.c
 *
 *  Created on: Nov 27, 2023
 *      Author: Calvin
 */

#include "task_main.h"

#include <stdint.h>
#include <stdbool.h>

#include "cmsis_os.h"
#include "tim.h"

#include "can_rtos.h"
#include "comms.h"
#include "sensor.h"
#include "heartbeat.h"
#include "state_machine.h"
#include "task_counter.h"
#include "inv_sevcon.h"
#include "p_12vSW.h"

#include "iwdg.h"

osThreadId_t th_tsk_main;
static uint64_t stk_tsk_main[32 * 4]; // 256*4
static StaticTask_t cb_tsk_main;
const osThreadAttr_t attr_tsk_main = { .name = "MAIN_TASK", .stack_size =
		sizeof(stk_tsk_main), .stack_mem = stk_tsk_main, .cb_size =
		sizeof(cb_tsk_main), .cb_mem = &cb_tsk_main, .priority =
		(osPriority_t) osPriorityHigh };
void taskMain(void *argument);

state_machine_t state_machine;

void setup_task_main() {
	// statically allocate all memory
	th_tsk_main = osThreadNew(taskMain, NULL, &attr_tsk_main);

	can_rtos_setup();

	// setup heartbeat mutexes and storage
	heartbeat_setup();

	// initialize state
	state_machine_init(&state_machine);
}

bool check_msg(can_msg_t *msg) {
#if VCU_CURRENT_ID == VCU_ID_CTRL

#if QEV4 == 1
	if ((msg->id & SEV_MASK_COMMON) == SEV_ID_COMMON) {
		if (inv_sevcon_check_msg(msg)) {
			return true;
		}
	}
#endif
#endif
	if ((msg->id & CAN_MASK_TYPE)
			== (CAN_TYPE_HEARTBEAT << CAN_ID_BIT_SHIFT_TYPE)) {
		if (heartbeat_check_msg(msg, &heartbeat_states, &heartbeat_boards)) {
			return true;
		}
	}

	return false;
}

void taskMain(void *argument) {
	uint16_t counter = 0;

	// setup things
	sensor_setup();

	// setup CAN
#if QEV4 == 1
	if (!init_CAN1()) {
		// TODO: do something lol
	}

	if (!init_CAN2()) {
		// TODO: do something lol
	}
#endif

#if QEV3 == 1
	if (!init_CAN1()) {
		// TODO: do something lol
	}
#endif

	// start timer
	HAL_TIM_Base_Start_IT(&htim5);

	uint32_t flags;

	// reset heartbeat timers -> from here on bad heartbeats is a fault
	heartbeat_mutex_acquire();
	heartbeat_timeout_reset(&heartbeat_states);
	heartbeat_mutex_release();

	for (;;) {
		flags = osThreadFlagsGet();

		// process sensors every 100Hz
		if ((flags & MAIN_EVT_FLAG_SENSOR) != 0) {
			osThreadFlagsClear(MAIN_EVT_FLAG_SENSOR);
			// process sensor data
			sensor_sample();
		}

		if ((flags & MAIN_EVT_FLAG_TIMER) != 0) {
			osThreadFlagsClear(MAIN_EVT_FLAG_TIMER);

			// increment task counter
			task_counter_mutex_acquire();
			task_counters.t_main++;
			task_counter_mutex_release();

//			// process CAN TX
			can_process_tx();

			heartbeat_mutex_acquire();

			// update comms
			comms_tx_timer();

			// process CAN messages
			can_msg_t can_msg;
			uint8_t count = osMessageQueueGetCount(q_can_rx);
			if (count > 0) {
				for (uint8_t i = 0; i < count; i++) {
					if (osMessageQueueGet(q_can_rx, &can_msg, NULL, 0)
							== osOK) {

						if (!check_msg(&can_msg)) {
							VCU_STATE new_state = state_machine_handle_CAN(
									&state_machine, &can_msg);
							if (new_state != state_machine.state_current) {
								(void) state_machine_change_state(
										&state_machine, new_state);
							}
						}
					}
					else {
						break;
					}
				}
			}

			// update heartbeat stuff
			if (!heartbeat_timeout_check(&heartbeat_states)) {
				// something important has timed out, so should transition into an error state
				// TODO: do something here lmao
				// lets go back to board check for now ONLY if we arent in shutdown, or an error state
				if ((state_machine.state_current > VCU_STATE_BOARD_CHECK)
						&& ((state_machine.state_current != VCU_STATE_SHUTDOWN)
								&& (state_machine.state_current
										!= VCU_STATE_ERROR)
								&& (state_machine.state_current
										!= VCU_STATE_TS_ERROR)
								&& (state_machine.state_current
										!= VCU_STATE_DVL_EMERGENCY))) {
					state_machine_change_state(&state_machine,
							VCU_STATE_BOARD_CHECK);
				}
			}

			// update state machine
			state_machine_update(&state_machine);

			// update outputs
			state_machine_set_outputs(&state_machine);

			heartbeat_mutex_release();

			if ((counter % 5) == 0) {
				// update s l o  w PWM @ 100 Hz
				pwm12V_timer_cb();
			}

			// increment counter
			counter = (counter + 1) % 500;
		}

	}

	// somehow exited, terminate task
	osThreadTerminate(NULL);
}

void main_tim_500Hz_interrupt_cb(void) {
//	HAL_GPIO_TogglePin(PMOD_CS_GPIO_Port, PMOD_CS_Pin);
	osThreadFlagsSet(th_tsk_main, MAIN_EVT_FLAG_TIMER);
}
