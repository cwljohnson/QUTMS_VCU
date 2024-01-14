/*
 * heartbeat.h
 *
 *  Created on: Jan 18, 2022
 *      Author: Calvin
 */

#ifndef INC_HEARTBEAT_H_
#define INC_HEARTBEAT_H_

#include <stdbool.h>
#include <CAN_BMU.h>
#include <CAN_MCISO.h>
#include <CAN_VCU.h>
#include <CAN_DVL.h>
#include <CAN_SW.h>
#include <CAN_RES.h>
#include <Timer.h>

#include "main.h"

typedef struct heartbeat_states {
	uint32_t heartbeat_timeout;

	bool BMU;
	bool MCISO[MCISO_COUNT];
	bool VCU[MAX_VCU];

	bool DVL_CTRL;

	uint32_t hb_DVL_start;

	uint32_t hb_BMU_start;
	uint32_t hb_MCISO_start[MCISO_COUNT];
	uint32_t hb_VCU_start[MAX_VCU];
} heatbeat_states_t;

extern heatbeat_states_t heartbeats;
extern ms_timer_t timer_heartbeat;

extern BMU_HeartbeatState_t BMU_hbState;
extern MCISO_HeartbeatState_t MCISO_hbState[MCISO_COUNT];
extern SW_HeartbeatState_t SW_hbState;

extern VCU_HeartbeatState_t VCU_heartbeatState;

extern VCU_HeartbeatState_t VCU_hbState_CTRL;
extern VCU_HeartbeatState_t VCU_hbState_DASH;
extern VCU_HeartbeatState_t VCU_hbState_SHDN;
extern VCU_HeartbeatState_t VCU_hbState_EBS;
extern VCU_HeartbeatState_t VCU_hbState_EBS_ADC;

extern RES_Status_t RES_hbState;

extern DVL_HeartbeatState_t DVL_hbState;

void setup_heartbeat();
void heartbeat_timer_cb(void *args);

void heartbeat_timeout_reset();

// call every time checking CAN message queue to update heartbeat status of boards
bool check_heartbeat_msg(CAN_MSG_Generic_t *msg);

// call to update status of heartbeat timeouts and detect potential board loss
bool check_bad_heartbeat();

#endif /* INC_HEARTBEAT_H_ */
