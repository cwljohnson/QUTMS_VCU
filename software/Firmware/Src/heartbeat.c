/*
 * heartbeat.c
 *
 *  Created on: Jan 18, 2022
 *      Author: Calvin
 */

#include "heartbeat.h"
#include "main.h"
#include "can.h"

#include <QUTMS_CAN.h>
#include "CAN_BMU.h"
#include <stdio.h>

#include "s_ctrl_steering.h"
#include "s_dashASB.h"

heatbeat_states_t heartbeats;
ms_timer_t timer_heartbeat;

BMU_HeartbeatState_t BMU_hbState;
MCISO_HeartbeatState_t MCISO_hbState[MCISO_COUNT];

VCU_HeartbeatState_t VCU_hbState_CTRL;
VCU_HeartbeatState_t VCU_hbState_DASH;
VCU_HeartbeatState_t VCU_hbState_SHDN;
VCU_HeartbeatState_t VCU_hbState_EBS;
VCU_HeartbeatState_t VCU_hbState_EBS_ADC;

RES_Status_t RES_hbState;

DVL_HeartbeatState_t DVL_hbState;

SW_HeartbeatState_t SW_hbState;

VCU_HeartbeatState_t VCU_heartbeatState;

uint32_t heartbeat_print_timer_start;

void setup_heartbeat() {
	// send heartbeat every 50ms
	timer_heartbeat = timer_init(50, true, heartbeat_timer_cb);

	// setup constants
	heartbeats.heartbeat_timeout = HEARTBEAT_TIMEOUT;

	// reset heartbeat timers to default
	heartbeat_timeout_reset();

	// start timer
	timer_start(&timer_heartbeat);

	heartbeat_print_timer_start = HAL_GetTick();
}

void heartbeat_timer_cb(void *args) {
	VCU_Heartbeat_t msg = Compose_VCU_Heartbeat(VCU_CURRENT_ID, &VCU_heartbeatState);
	CAN_TxHeaderTypeDef header = { .ExtId = msg.id, .IDE =
	CAN_ID_EXT, .RTR = CAN_RTR_DATA, .DLC = sizeof(msg.data), .TransmitGlobalTime = DISABLE };

	// send heartbeat
	send_can_msg(&hcan1, &header, msg.data);

	if ((HAL_GetTick() - heartbeat_print_timer_start) > HEARTBEAT_PRINT_TIME) {
		heartbeat_print_timer_start = HAL_GetTick();

		printf("HB: State: 0x%02X, Core Flags: 0x%04X Flags: 0x%04X\r\n", VCU_heartbeatState.stateID,
				VCU_heartbeatState.coreFlags.rawMem, VCU_heartbeatState.otherFlags.rawMem);
	}

}

void heartbeat_timeout_reset() {

}

// call every time checking CAN message queue to update heartbeat status of boards
bool check_heartbeat_msg(CAN_MSG_Generic_t *msg) {
	bool hb_message = false;

	uint8_t idx = (msg->ID & 0xF);
	uint32_t masked_id = (msg->ID & ~0xF);

	if (masked_id == MCISO_Heartbeat_ID) {
		hb_message = true;

		// check index is in correct range
		if (idx < MCISO_COUNT) {
			heartbeats.hb_MCISO_start[idx] = HAL_GetTick();
			heartbeats.MCISO[idx] = true;

			// update heartbeat struct
			Parse_MCISO_Heartbeat(msg->data, &MCISO_hbState[idx]);
		}
	}
	else if (masked_id == BMU_Heartbeat_ID) {
		hb_message = true;

		heartbeats.hb_BMU_start = HAL_GetTick();
		heartbeats.BMU = true;

#if VCU_CURRENT_ID == VCU_ID_CTRL
		// have heartbeat so clear error flag if it's set
		VCU_heartbeatState.otherFlags.ctrl._VCU_Flags_Ctrl.HB_BMU = 0;
#elif VCU_CURRENT_ID == VCU_ID_DASH
		// have heartbeat so clear error flag if it's set
		VCU_heartbeatState.otherFlags.dash._VCU_Flags_Dash.HB_BMU = 0;
#endif

		Parse_BMU_Heartbeat(msg->data, &BMU_hbState);
	}
	else if (masked_id == VCU_Heartbeat_ID) {
		hb_message = true;

		if (idx < MAX_VCU) {
			heartbeats.hb_VCU_start[idx] = HAL_GetTick();
			heartbeats.VCU[idx] = true;

			VCU_heartbeatState.VCU |= (1 << idx);

			// update heartbeat structs
			if (idx == VCU_ID_CTRL) {
				Parse_VCU_Heartbeat(msg->data, &VCU_hbState_CTRL);
			}
			else if (idx == VCU_ID_DASH) {
				Parse_VCU_Heartbeat(msg->data, &VCU_hbState_DASH);

#if VCU_CURRENT_ID == VCU_ID_CTRL
				ctrl_steering_values.steering_imp_present = VCU_hbState_DASH.otherFlags.dash._VCU_Flags_Dash.IMP_Steer
						== 1;
#endif

			}
			else if (idx == VCU_ID_SHDN) {
				Parse_VCU_Heartbeat(msg->data, &VCU_hbState_SHDN);

#if VCU_CURRENT_ID == VCU_ID_DASH
				// valid heartbeat so clear error flag if set
				VCU_heartbeatState.otherFlags.dash._VCU_Flags_Dash.HB_VCU_SHDN = 0;
#endif
			}
			else if (idx == VCU_ID_EBS) {
				Parse_VCU_Heartbeat(msg->data, &VCU_hbState_EBS);
			}
			else if (idx == VCU_ID_EBS_ADC) {
				Parse_VCU_Heartbeat(msg->data, &VCU_hbState_EBS_ADC);

#if VCU_CURRENT_ID == VCU_ID_EBS
				VCU_heartbeatState.otherFlags.ebs._VCU_Flags_EBS.HB_EBS_ADC = 0;

				VCU_heartbeatState.otherFlags.ebs._VCU_Flags_EBS.DET_24V = VCU_hbState_EBS_ADC.otherFlags.ebs_adc._VCU_Flags_EBS_ADC.DET_24V;
				VCU_heartbeatState.otherFlags.ebs._VCU_Flags_EBS.DET_BTN = VCU_hbState_EBS_ADC.otherFlags.ebs_adc._VCU_Flags_EBS_ADC.DET_BTN;
				VCU_heartbeatState.otherFlags.ebs._VCU_Flags_EBS.DET_PWR_EBS = VCU_hbState_EBS_ADC.otherFlags.ebs_adc._VCU_Flags_EBS_ADC.DET_PWR_EBS;
#endif
			}
		}
	}
	else if (masked_id == SW_Heartbeat_ID) {
		hb_message = true;
		Parse_SW_Heartbeat(msg->data, &SW_hbState);
	}
	else if (masked_id == DVL_Heartbeat_ID) {
		heartbeats.hb_DVL_start = HAL_GetTick();
		heartbeats.DVL_CTRL = true;

#if VCU_CURRENT_ID == VCU_ID_EBS
		VCU_heartbeatState.otherFlags.ebs._VCU_Flags_EBS.HB_DVL = 0;
#endif

		hb_message = true;
		Parse_DVL_Heartbeat(msg->data, &DVL_hbState);
	}
	else if (msg->ID == RES_Heartbeat_ID) {
		hb_message = true;
		Parse_RES_Heartbeat(msg->data, &RES_hbState);

#if (VCU_CURRENT_ID == VCU_ID_CTRL) && (DRIVERLESS_CTRL == 1)
		if (RES_hbState.estop) {
			// light on
			dashASB_setState(true);
		} else {
			// light off
			dashASB_setState(false);
		}
#endif
	}

	// check all MCISO boards for valid heartbeats
	bool mciso_hb_good = true;
	for (int i = 0; i < MCISO_COUNT; i++) {
		if (!heartbeats.MCISO[i]) {
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

	return hb_message;
}

// call to update status of heartbeat timeouts and detect potential board loss
bool check_bad_heartbeat() {
	bool success = true;

	// BMU
	if ((HAL_GetTick() - heartbeats.hb_BMU_start) > heartbeats.heartbeat_timeout) {
		heartbeats.BMU = false;

#if VCU_CURRENT_ID == VCU_ID_CTRL
		VCU_heartbeatState.otherFlags.ctrl._VCU_Flags_Ctrl.HB_BMU = 1;
#elif VCU_CURRENT_ID == VCU_ID_DASH
		VCU_heartbeatState.otherFlags.dash._VCU_Flags_Dash.HB_BMU = 1;
#endif

#if (DEBUG_BMU == 0) && (DEBUG_HB_FAIL == 0)
		success = false;
#endif
	}

	// MCISO
	for (int i = 0; i < MCISO_COUNT; i++) {
		if ((HAL_GetTick() - heartbeats.hb_MCISO_start[i]) > heartbeats.heartbeat_timeout) {
			heartbeats.MCISO[i] = false;

#if VCU_CURRENT_ID == VCU_ID_CTRL
			VCU_heartbeatState.otherFlags.ctrl._VCU_Flags_Ctrl.HB_MCISO = 1;
#endif

#if (DEBUG_MCISO == 0) && (DEBUG_HB_FAIL == 0)
			success = false;
#endif
		}
	}

	// VCU
	for (int i = 0; i < MAX_VCU; i++) {
		if ((HAL_GetTick() - heartbeats.hb_VCU_start[i]) > heartbeats.heartbeat_timeout) {
			heartbeats.VCU[i] = false;
			VCU_heartbeatState.VCU &= ~(1 << i);
		}
	}

#if VCU_CURRENT_ID == VCU_ID_DASH
	if (!heartbeats.VCU[VCU_ID_SHDN]) {
		VCU_heartbeatState.otherFlags.dash._VCU_Flags_Dash.HB_VCU_SHDN = 1;
		success = false;
	}
#endif

#if VCU_CURRENT_ID == VCU_ID_EBS
	if (!heartbeats.VCU[VCU_ID_EBS_ADC]) {
		VCU_heartbeatState.otherFlags.ebs._VCU_Flags_EBS.HB_EBS_ADC = 1;
		success = false;
	}
#endif

	// DVL
	if ((HAL_GetTick() - heartbeats.hb_DVL_start) > heartbeats.heartbeat_timeout) {
		heartbeats.DVL_CTRL = false;

#if VCU_CURRENT_ID == VCU_ID_EBS
		VCU_heartbeatState.otherFlags.ebs._VCU_Flags_EBS.HB_DVL = 1;
#endif
	}

	return success;
}
