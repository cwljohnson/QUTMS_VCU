/*
 * comms.h
 *
 *  Created on: 22 May 2023
 *      Author: Calvin
 */

#ifndef INC_COMMS_H_
#define INC_COMMS_H_

#include "can_rtos.h"
#include <stdbool.h>

#define TX_TIMER_1HZ_COUNT 500
#define TX_TIMER_10HZ_COUNT 50
#define TX_TIMER_50HZ_COUNT 10
#define TX_TIMER_100HZ_COUNT 5

void comms_tx_timer();

#endif /* INC_COMMS_H_ */
