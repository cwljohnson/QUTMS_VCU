/*
 * inv_vesc.c
 *
 *  Created on: Dec 11, 2023
 *      Author: Calvin
 */

#include "inv_vesc.h"
#include <string.h>

#include "cmsis_os.h"
#include "heartbeat.h"

#include <CAN_VESC.h>

#if QEV3 == 1

inv_vesc_t inverters;

void inv_vesc_setup(void) {
	memset(&inverters, 0, sizeof(inv_vesc_t));

	// motor IDs are as follows

	// FL - 0
	// FR - 1
	// RL - 2
	// RR - 3

	// 120 A
	inverters.max_current = 120;
	inverters.max_regen_current = 60;
	inverters.regen_kmh_cutoff = 10;
	for (uint8_t i = 0; i < 4; i++) {
		inverters.vesc[i].id = i;
	}
}

void inv_vesc_tx(void) {
	uint8_t currentState = VCU_heartbeatState.stateID;

	if ((currentState == VCU_STATE_SHUTDOWN)
			|| (currentState == VCU_STATE_TS_ERROR)) {
		// shutdown state, request a shutdown
		for (uint8_t i = 0; i < NUM_VESC; i++) {
			inv_vesc_tx_shdn(i);
		}
	}
	else if ((currentState == VCU_STATE_DRIVING)
			|| (currentState == VCU_STATE_DVL_DRIVING)) {
		// driving state, send torque commands
		for (uint8_t i = 0; i < NUM_VESC; i++) {
			inv_vesc_tx_req(&inverters.vesc[i]);
		}
	}
}

void inv_vesc_tx_shdn(uint8_t id) {
	VESC_Shutdown_t shutdown_msg = Compose_VESC_Shutdown(id);
	can_msg_t msg;
	msg.ide = CAN_ID_EXT;
	msg.id = shutdown_msg.id;
	msg.dlc = 0;

	can_tx_enqueue(&msg, CAN_SRC_CTRL);

}

void inv_vesc_tx_req(vesc_t *inverter) {
	can_msg_t msg;
	msg.ide = CAN_ID_EXT;

	// TODO: put regen swapping here (eg send braking command instead)
	float current_request = inverter->current_request;

	if (current_request >= 0) {
		VESC_SetCurrent_t req_msg = Compose_VESC_SetCurrent(inverter->id,
				current_request);

		msg.id = req_msg.id;
		msg.dlc = sizeof(req_msg.data);
		memcpy(msg.data, req_msg.data, msg.dlc);

		can_tx_enqueue(&msg, CAN_SRC_CTRL);

	} else {
		// negative, so braking
		VESC_SetCurrentBrake_t req_msg = Compose_VESC_SetCurrentBrake(inverter->id,
				-current_request);

		msg.id = req_msg.id;
		msg.dlc = sizeof(req_msg.data);
		memcpy(msg.data, req_msg.data, msg.dlc);

		can_tx_enqueue(&msg, CAN_SRC_CTRL);
	}



}

#endif
