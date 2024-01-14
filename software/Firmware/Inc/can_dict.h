/*
 * can_dict.h
 *
 *  Created on: 2 Feb. 2022
 *      Author: Calvin J
 */

#ifndef INC_CAN_DICT_H_
#define INC_CAN_DICT_H_

#include <obj_dic.h>
#include <QUTMS_CAN.h>
#include <Timer.h>

extern obj_dict_t VCU_obj_dict;
extern ms_timer_t timer_OD;

void VCU_OD_init();
void VCU_OD_handleCAN(CAN_MSG_Generic_t *msg);
void OD_timer_cb(void *args);

#endif /* INC_CAN_DICT_H_ */
