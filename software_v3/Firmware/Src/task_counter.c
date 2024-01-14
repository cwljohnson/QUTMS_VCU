/*
 * task_counter.c
 *
 *  Created on: Aug 8, 2023
 *      Author: Calvin
 */

#include "task_counter.h"
#include "tasks.h"

#include "cmsis_os.h"


#include <string.h>

task_counters_t task_counters;

osMutexId_t mtx_task_counter;
static StaticSemaphore_t cb_mtx_task_counter;
const osMutexAttr_t attr_mtx_task_counter = { .cb_size = sizeof(cb_mtx_task_counter), .cb_mem = &cb_mtx_task_counter,
		.attr_bits =
		osMutexPrioInherit };

void task_counter_setup(void) {
	memset(&task_counters, 0, sizeof(task_counters));

	mtx_task_counter = osMutexNew(&attr_mtx_task_counter);
}

void task_counter_mutex_acquire(void) {
	osMutexAcquire(mtx_task_counter, osWaitForever);
}

void task_counter_mutex_release(void) {
	osMutexRelease(mtx_task_counter);
}

bool counter_check(uint32_t counter_value, uint32_t exp_per_us) {
	uint32_t expected_value = TASK_COUNTER_CHECK_TICKS_US / exp_per_us;
	uint32_t min_value = ((expected_value * 100) - (expected_value * TASK_COUNTER_CHECK_ACCURACY)) / 100;

	return (counter_value > min_value);
}

bool task_counter_check(task_counters_t *counter) {
	bool counters_good = true;

	if (!counter_check(counter->t_main, TASK_MAIN_EXP_PER_US)) {
		counters_good = false;
		counter->failed_flags |= (1 << TASK_ID_MAIN);
	}

	if (!counter_check(counter->t_spi, TASK_SPI_EXP_PER_US)) {
		counters_good = false;
		counter->failed_flags |= (1 << TASK_ID_SPI);
	}

	counter->t_main = 0;
	counter->t_spi = 0;

	return counters_good;
}
