/*
 * VCU_shutdown.h
 *
 *  Created on: 22 Nov. 2021
 *      Author: Calvin
 */

#ifndef INC_VCU_SHUTDOWN_H_
#define INC_VCU_SHUTDOWN_H_

#include <Timer.h>

extern ms_timer_t timer_shutdown;
extern bool shutdown_state;
extern bool shutdown_segments[16];

void setup_shutdown_timer();
void shutdown_timer_cb(void *args);

#endif /* INC_VCU_SHUTDOWN_H_ */
