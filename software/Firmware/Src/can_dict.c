/*
 * can_dict.c
 *
 *  Created on: 2 Feb. 2022
 *      Author: Calvin
 */

#include "can_dict.h"
#include "can.h"

#include <CAN_VCU.h>

#include "inverter.h"

obj_dict_t VCU_obj_dict;
ms_timer_t timer_OD;

void VCU_OD_init() {
	OD_init(&VCU_obj_dict);

#if VCU_CURRENT_ID == VCU_ID_CTRL
	inverter_setup();

	OD_initValue(&VCU_obj_dict, VCU_OD_ID_CTRL_REGEN_MAX_CURRENT, OD_TYPE_UINT16, &inverter_config.regen_max_current);
#endif

	// place holder to remind me
	//OD_setValue(&CC_obj_dict, CC_OD_IDX_INV_CURRENT, VESC_CURRENT_MAX);

	// object dictionary messages are fairly quiet, so only need to check every 25ms
	timer_OD = timer_init(25, true, OD_timer_cb);

	// start timer
	timer_start(&timer_OD);
}

void VCU_OD_handleCAN(CAN_MSG_Generic_t *msg) {
	uint8_t outputData[8];

	// interprets CAN message as either get value
	bool sendMsg = OD_handleCAN(&VCU_obj_dict, msg->data, outputData);

	if (sendMsg) {
		CAN_TxHeaderTypeDef header = { .IDE = CAN_ID_EXT, .RTR = CAN_RTR_DATA, .TransmitGlobalTime = DISABLE };

		VCU_OBJ_DICT_t new_msg = Compose_VCU_OBJ_DICT(VCU_CURRENT_ID, outputData);
		header.ExtId = new_msg.id;
		header.DLC = sizeof(new_msg.data);

		// send VCU CAN
		send_can_msg((CAN_HandleTypeDef *)msg->hcan, &header, new_msg.data);
	}
}

void OD_timer_cb(void *args) {
	CAN_MSG_Generic_t msg;

	while (queue_next(&queue_CAN_OD, &msg)) {
		VCU_OD_handleCAN(&msg);
	}
}
