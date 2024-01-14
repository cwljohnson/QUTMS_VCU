/*
 * heartbeat.c
 *
 *  Created on: 8 May 2023
 *      Author: Calvin
 */

#include "heartbeat.h"
#include "can_rtos.h"

#include "default_config.h"
#include "main.h"

#include <QUTMS_can.h>

#include "cmsis_os.h"

heartbeat_config_t heartbeat_config;
heartbeat_states_t heartbeat_states;
heartbeat_boards_t heartbeat_boards;

VCU_HeartbeatState_t VCU_heartbeatState;

osMutexId_t mtx_heartbeat;
static StaticSemaphore_t cb_mtx_heartbeat;
const osMutexAttr_t attr_mtx_heartbeat = { .cb_size = sizeof(cb_mtx_heartbeat),
		.cb_mem = &cb_mtx_heartbeat, .attr_bits = osMutexPrioInherit };

void heartbeat_setup(void) {
	// TODO: this should be handled by config or some shit
	heartbeat_config.heartbeat_timeout = HEARTBEAT_TIMEOUT;

	// clear heartbeat flags
	VCU_heartbeatState.coreFlags.rawMem = 0;
	VCU_heartbeatState.otherFlags.rawMem = 0;

	mtx_heartbeat = osMutexNew(&attr_mtx_heartbeat);
}

void heartbeat_transmit(void) {
	// grab heartbeat info
	VCU_Heartbeat_t hb_msg = Compose_VCU_Heartbeat(VCU_CURRENT_ID,
			&VCU_heartbeatState);

	can_msg_t msg;
	msg.id = hb_msg.id;
	msg.dlc = sizeof(hb_msg.data);
	msg.ide = CAN_ID_EXT;

	for (uint8_t i = 0; i < msg.dlc; i++) {
		msg.data[i] = hb_msg.data[i];
	}

	can_tx_enqueue(&msg, CAN_SRC_CTRL);
}

void heartbeat_timeout_reset(heartbeat_states_t *hb_state) {
	uint32_t current_tick = osKernelGetTickCount();

	// BMU
	hb_state->hb_BMU.hb_start = current_tick;
	hb_state->hb_BMU.hb_alive = false;

	// VCU CTRL
	hb_state->hb_VCU_CTRL.hb_start = current_tick;
	hb_state->hb_VCU_CTRL.hb_alive = false;

#if QEV3 == 1
	for (uint8_t i = 0; i < MCISO_COUNT; i++) {
		hb_state->hb_MCISO[i].hb_start = current_tick;
		hb_state->hb_MCISO[i].hb_alive = false;
	}

	// VCU CTRL
	hb_state->hb_VCU_EBS_BTN.hb_start = current_tick;
	hb_state->hb_VCU_EBS_BTN.hb_alive = false;

	hb_state->hb_EBS_CTRL.hb_start = current_tick;
	hb_state->hb_EBS_CTRL.hb_alive = false;

	hb_state->hb_DVL.hb_start = current_tick;
	hb_state->hb_DVL.hb_alive = false;
#endif
}

bool heartbeat_timeout_check(heartbeat_states_t *hb_state) {
	bool success = true;
	uint32_t current_tick = osKernelGetTickCount();

	// BMU
	if ((current_tick - hb_state->hb_BMU.hb_start)
			> heartbeat_config.heartbeat_timeout) {
		hb_state->hb_BMU.hb_alive = false;

#if VCU_CURRENT_ID == VCU_ID_CTRL
		VCU_heartbeatState.otherFlags.ctrl._VCU_Flags_Ctrl.HB_BMU = 1;

#if DEBUG_BMU == 0
		success = false;
#endif
#endif

	}

	if ((current_tick - hb_state->hb_VCU_CTRL.hb_start)
			> heartbeat_config.heartbeat_timeout) {
		hb_state->hb_VCU_CTRL.hb_alive = false;

#if (VCU_CURRENT_ID == VCU_ID_ACCU)
		VCU_heartbeatState.otherFlags.accu._VCU_Flags_ACCU.HB_VCU_CTRL = 1;
#endif
	}

#if QEV3 == 1
	// EBS
	if ((current_tick - hb_state->hb_EBS_CTRL.hb_start)
			> heartbeat_config.heartbeat_timeout) {
		hb_state->hb_EBS_CTRL.hb_alive = false;

#if (VCU_CURRENT_ID == VCU_ID_CTRL)
#if DRIVERLESS_CTRL == 1
		success = false;
#endif
		VCU_heartbeatState.otherFlags.ctrl._VCU_Flags_Ctrl.HB_EBS = 1;
#endif
	}

	// VCU EBS BTN
	if ((current_tick - hb_state->hb_VCU_EBS_BTN.hb_start)
			> heartbeat_config.heartbeat_timeout) {
		hb_state->hb_VCU_EBS_BTN.hb_alive = false;

#if (VCU_CURRENT_ID == VCU_ID_CTRL)
		VCU_heartbeatState.otherFlags.ctrl._VCU_Flags_Ctrl.HB_VCU_EBS_BTN = 1;
#endif
	}

	// DVL
	if ((current_tick - hb_state->hb_DVL.hb_start)
			> heartbeat_config.heartbeat_timeout) {
		hb_state->hb_DVL.hb_alive = false;

#if (VCU_CURRENT_ID == VCU_ID_CTRL)
		// this is never a failure because they like to turn the DVL PC off all the time lmao
		VCU_heartbeatState.otherFlags.ctrl._VCU_Flags_Ctrl.HB_DVL = 1;
#endif
	}

	// MCISO
	for (uint8_t i = 0; i < MCISO_COUNT; i++) {
		if ((current_tick - hb_state->hb_MCISO[i].hb_start)
				> heartbeat_config.heartbeat_timeout) {
			hb_state->hb_MCISO[i].hb_alive = false;

#if (VCU_CURRENT_ID == VCU_ID_CTRL)
			success = false;
			VCU_heartbeatState.otherFlags.ctrl._VCU_Flags_Ctrl.HB_MCISO = 1;
#endif
		}
	}
#endif

	return success;
}

bool heartbeat_check_msg(can_msg_t *msg, heartbeat_states_t *hb_state,
		heartbeat_boards_t *hb_board) {
	bool success = false;
	uint32_t current_tick = osKernelGetTickCount();

	uint32_t masked_id = (msg->id & ~0xF);
	uint8_t idx = (msg->id & 0xF);

	if (msg->id == BMU_Heartbeat_ID) {
		success = true;

		hb_state->hb_BMU.hb_start = current_tick;
		hb_state->hb_BMU.hb_alive = true;

#if VCU_CURRENT_ID == VCU_ID_CTRL
		VCU_heartbeatState.otherFlags.ctrl._VCU_Flags_Ctrl.HB_BMU = 0;
#endif

		Parse_BMU_Heartbeat(msg->data, &hb_board->BMU_hbState);
	}

	else if (msg->id == (VCU_Heartbeat_ID + VCU_ID_CTRL)) {
		success = true;

		hb_state->hb_VCU_CTRL.hb_start = current_tick;
		hb_state->hb_VCU_CTRL.hb_alive = true;

#if (VCU_CURRENT_ID == VCU_ID_ACCU)
		VCU_heartbeatState.otherFlags.accu._VCU_Flags_ACCU.HB_VCU_CTRL = 0;
#endif

		Parse_VCU_Heartbeat(msg->data, &hb_board->VCU_CTRL_hbState);
	}

#if QEV3 == 1
	else if (msg->id == EBS_CTRL_Heartbeat_ID) {
		success = true;

		hb_state->hb_EBS_CTRL.hb_start = current_tick;
		hb_state->hb_EBS_CTRL.hb_alive = true;

#if VCU_CURRENT_ID == VCU_ID_CTRL
		VCU_heartbeatState.otherFlags.ctrl._VCU_Flags_Ctrl.HB_EBS = 0;
#endif

		Parse_EBS_CTRL_Heartbeat(msg->data, &hb_board->EBS_CTRL_hbState);
	}
	else if (msg->id == (VCU_Heartbeat_ID + VCU_ID_EBS_BTN)) {
			success = true;

			hb_state->hb_VCU_EBS_BTN.hb_start = current_tick;
			hb_state->hb_VCU_EBS_BTN.hb_alive = true;

	#if (VCU_CURRENT_ID == VCU_ID_CTRL)
			VCU_heartbeatState.otherFlags.ctrl._VCU_Flags_Ctrl.HB_VCU_EBS_BTN = 0;
	#endif

			Parse_VCU_Heartbeat(msg->data, &hb_board->VCU_EBS_BTN_hbState);
		}
	else if (masked_id == MCISO_Heartbeat_ID) {
		success = true;

		if (idx < MCISO_COUNT) {
			hb_state->hb_MCISO[idx].hb_start = current_tick;
			hb_state->hb_MCISO[idx].hb_alive = true;

			Parse_MCISO_Heartbeat(msg->data, &hb_board->MCISO_hbState[idx]);
		}

		// check all MCISO boards for valid heartbeats
		bool mciso_hb_good = true;
		for (int i = 0; i < MCISO_COUNT; i++) {
			if (!hb_state->hb_MCISO[i].hb_alive) {
				mciso_hb_good = false;
				break;
			}
		}

		if (mciso_hb_good) {
#if VCU_CURRENT_ID == VCU_ID_CTRL
			// valid heartbeats for all MCISO boards so clear error flag if it's set
			VCU_heartbeatState.otherFlags.ctrl._VCU_Flags_Ctrl.HB_MCISO = 0;
#endif
		}

	}
	else if (masked_id == DVL_Heartbeat_ID) {
		success = true;
		hb_state->hb_DVL.hb_start = current_tick;
		hb_state->hb_DVL.hb_alive = true;

#if VCU_CURRENT_ID == VCU_ID_CTRL
		VCU_heartbeatState.otherFlags.ctrl._VCU_Flags_Ctrl.HB_DVL = 0;
#endif

		Parse_DVL_Heartbeat(msg->data, &hb_board->DVL_hbState);
	}
	else if (masked_id == RES_Heartbeat_ID) {
		success = true;

		Parse_RES_Heartbeat(msg->data, &hb_board->RES_hbState);

		// TODO: update dash ASB light somewhere?????????
	}
#endif

	return success;
}

void heartbeat_mutex_acquire(void) {
	osMutexAcquire(mtx_heartbeat, osWaitForever);
}

void heartbeat_mutex_release(void) {
	osMutexRelease(mtx_heartbeat);
}
