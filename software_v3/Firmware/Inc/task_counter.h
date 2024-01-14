/*
 * task_counter.h
 *
 *  Created on: Aug 8, 2023
 *      Author: Calvin
 */

#ifndef INC_TASK_COUNTER_H_
#define INC_TASK_COUNTER_H_

#include <stdint.h>
#include <stdbool.h>

#include "task_spi.h"

typedef struct task_counters {
	uint32_t t_main;
	uint32_t t_spi;

	uint32_t failed_flags;
} task_counters_t;

extern task_counters_t task_counters;

// expected period in us
// 1 kHz
#define TASK_MAIN_EXP_PER_US (500)
// 10 kHz
#define TASK_SPI_EXP_PER_US (100)

#define TASK_COUNTER_CHECK_TICKS (100)
#define TASK_COUNTER_CHECK_TICKS_US (TASK_COUNTER_CHECK_TICKS * 1000)

// % accuracy -> 20 = 20%
#define TASK_COUNTER_CHECK_ACCURACY (10)

void task_counter_setup(void);
void task_counter_mutex_acquire(void);
void task_counter_mutex_release(void);

bool task_counter_check(task_counters_t *counter);

#endif /* INC_TASK_COUNTER_H_ */
