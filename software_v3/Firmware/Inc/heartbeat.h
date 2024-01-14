/*
 * heartbeat.h
 *
 *  Created on: 8 May 2023
 *      Author: Calvin
 */

#ifndef INC_HEARTBEAT_H_
#define INC_HEARTBEAT_H_

#include <stdint.h>
#include <stdbool.h>

#include <CAN_BMU.h>
#include <CAN_VCU.h>
#include "main.h"
#include "can_rtos.h"

#if QEV3 == 1
#include <CAN_MCISO.h>
#include <CAN_EBS_CTRL.h>
#include <CAN_DVL.h>
#include <CAN_RES.h>
#endif

typedef struct {
	uint32_t heartbeat_timeout;
} heartbeat_config_t;

typedef struct {
	uint32_t hb_start;
	bool hb_alive;
} hb_storage_t;

typedef struct {
	hb_storage_t hb_BMU;
	hb_storage_t hb_VCU_CTRL;

#if QEV3 == 1
	hb_storage_t hb_EBS_CTRL;
	hb_storage_t hb_VCU_EBS_BTN;
	hb_storage_t hb_MCISO[MCISO_COUNT];
	hb_storage_t hb_DVL;
#endif


} heartbeat_states_t;

typedef struct {
	BMU_HeartbeatState_t BMU_hbState;
	VCU_HeartbeatState_t VCU_CTRL_hbState;

#if QEV3 == 1
	VCU_HeartbeatState_t VCU_EBS_BTN_hbState;
	EBS_CTRL_HeartbeatState_t EBS_CTRL_hbState;
	MCISO_HeartbeatState_t MCISO_hbState[MCISO_COUNT];
	DVL_HeartbeatState_t DVL_hbState;
	RES_Status_t RES_hbState;
#endif
} heartbeat_boards_t;

extern heartbeat_config_t heartbeat_config;
extern heartbeat_states_t heartbeat_states;
extern heartbeat_boards_t heartbeat_boards;

extern VCU_HeartbeatState_t VCU_heartbeatState;

void heartbeat_setup(void);
void heartbeat_transmit(void);

void heartbeat_timeout_reset(heartbeat_states_t *hb_state);
bool heartbeat_timeout_check(heartbeat_states_t *hb_state);

bool heartbeat_check_msg(can_msg_t *msg, heartbeat_states_t *hb_state,
		heartbeat_boards_t *hb_board);

void heartbeat_mutex_acquire(void);
void heartbeat_mutex_release(void);

#endif /* INC_HEARTBEAT_H_ */
