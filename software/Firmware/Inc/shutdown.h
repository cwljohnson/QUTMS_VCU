/*
 * shutdown.h
 *
 *  Created on: Feb 9, 2022
 *      Author: Calvin
 */

#ifndef INC_SHUTDOWN_H_
#define INC_SHUTDOWN_H_

#include <QUTMS_CAN.h>
#include <stdbool.h>

bool check_shutdown_msg(CAN_MSG_Generic_t *msg, bool *shdn_triggered);

extern bool shutdown_status;
extern bool shutdown_reset;

#endif /* INC_SHUTDOWN_H_ */
