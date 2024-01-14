/*
 * tasks.h
 *
 *  Created on: Feb 6, 2023
 *      Author: Calvin
 */

#ifndef INC_TASKS_H_
#define INC_TASKS_H_

typedef enum {
	TASK_ID_MAIN,
	TASK_ID_SPI
} TASK_IDS;

void setup_tasks();

#endif /* INC_TASKS_H_ */
