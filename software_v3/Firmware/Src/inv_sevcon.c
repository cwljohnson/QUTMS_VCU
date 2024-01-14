/*
 * inv_sevcon.c
 *
 *  Created on: Nov 20, 2023
 *      Author: Calvin
 */

#include "inv_sevcon.h"

#include <CAN_SEVCON.h>
#include <string.h>

#include "cmsis_os.h"
#include "heartbeat.h"

#if QEV4 == 1

#define SPEED_LIMIT_HARD 13000
#define SPEED_LIMIT_SOFT 10500

inv_sevcon_t inverters;

void inv_sevcon_setup(void) {
	// NOTE: everything here is protected by the heartbeat mutex jazz

	memset(&inverters, 0, sizeof(inv_sevcon_t));

	inverters.sevcon[0].address = 0x71;
	inverters.sevcon[1].address = 0x72;

	// 3.0 Nm
	inverters.max_torque = 13.0;
	for (uint8_t i = 0; i < NUM_INV; i++) {
		inverters.sevcon[i].ctrl.torque_limit_drive = inverters.max_torque;
		inverters.sevcon[i].ctrl.torque_limit_regen = 0;
		inverters.sevcon[i].ctrl.speed_limit_forward = SPEED_LIMIT_HARD;
		inverters.sevcon[i].ctrl.speed_limit_soft = SPEED_LIMIT_SOFT;
		inverters.sevcon[i].ctrl.speed_limit_backward = -100;
		inverters.sevcon[i].ctrl.current_limit_discharge = 30;
		inverters.sevcon[i].ctrl.current_limit_charge = -30;
	}
}

bool inv_sevcon_check_msg(can_msg_t *msg) {

	bool match = false;

	for (uint8_t i = 0; i < NUM_INV; i++) {
		if ((msg->id & 0xFF) == inverters.sevcon[i].address) {
			// positive match on common bit and the source address
			// check specific PGN
			uint32_t pgn = ((msg->id >> 8) & 0x1FF00);

			if (pgn == SEVCON_PGN_HS1) {
				match = true;

				int16_t outputTorque;
				int16_t motorSpeed;
				int16_t batteryCurrent;

				Parse_Sevcon_HS1(msg->data, &outputTorque,
						&motorSpeed, &batteryCurrent);

				inverters.sevcon[i].data.motor_speed = motorSpeed;
			}
			else if (pgn == SEVCON_PGN_HS2) {
				match = true;
				int16_t availableTorqueF;
				int16_t availableTorqueR;

				Parse_Sevcon_HS2(msg->data, &availableTorqueF,
						&availableTorqueR,
						&inverters.sevcon[i].data.status_word);
			}

			if (match) {
				// great success reset timeout
				inverters.sevcon[i].alive = true;
				inverters.sevcon[i].last_time = osKernelGetTickCount();

				bool all_good = true;
				for (uint8_t j = 0; j < NUM_INV; j++) {
					all_good = all_good && inverters.sevcon[j].alive;
				}

				if (all_good) {
					VCU_heartbeatState.otherFlags.ctrl._VCU_Flags_Ctrl.HB_INV =
							0;
				}
			}
		}
	}

	return match;
}

void inv_sevcon_tx(void) {
	uint8_t currentState = VCU_heartbeatState.stateID;
	uint32_t timeout = heartbeat_config.heartbeat_timeout;

	// dear future whoever
	// this is probably not the best place to do the timeout check but i cbf and it'll work teehee
	uint32_t current_tick = osKernelGetTickCount();

	for (uint8_t i = 0; i < NUM_INV; i++) {
		if ((current_tick - inverters.sevcon[i].last_time) > timeout) {
			inverters.sevcon[i].alive = false;
			VCU_heartbeatState.otherFlags.ctrl._VCU_Flags_Ctrl.HB_INV = 1;
		}
	}

	if (!((currentState == VCU_STATE_INVERTER_CHECK)
			|| (currentState == VCU_STATE_INVERTER_ENERGIZE)
			|| (currentState == VCU_STATE_RTD_RDY)
			|| (currentState == VCU_STATE_RTD_BTN)
			|| (currentState == VCU_STATE_DRIVING)
			|| (currentState == VCU_STATE_TS_ERROR))) {
		// if we're not in any of the above states, don't really need to send anything lol
		return;
	}

	// game is game
	for (uint8_t i = 0; i < NUM_INV; i++) {
		inv_sevcon_tx_hc(&inverters.sevcon[i]);
	}
}

void inv_sevcon_tx_hc(sevcon_t *inverter) {
	sevcon_hs_t hs_msg;
	uint8_t source = 0x69;

	int16_t torque_demand = (inverter->ctrl.torque_demand * 16);
	int16_t torque_limit_drive = (inverter->ctrl.torque_limit_drive * 16);
	int16_t torque_limit_regen = (inverter->ctrl.torque_limit_regen * 16);
	int16_t target_cap_voltage = (inverter->ctrl.target_cap_voltage * 16);

	hs_msg = Compose_Sevcon_HC1(inverter->address, source, torque_demand,
			inverter->ctrl.control_word, torque_limit_drive);
	inv_sevcon_tx_msg(&hs_msg);

	hs_msg = Compose_Sevcon_HC2(inverter->address, source, torque_limit_regen,
			inverter->ctrl.speed_limit_forward,
			inverter->ctrl.speed_limit_backward);
	inv_sevcon_tx_msg(&hs_msg);

	hs_msg = Compose_Sevcon_HC3(inverter->address, source,
			inverter->ctrl.current_limit_discharge,
			inverter->ctrl.current_limit_charge, target_cap_voltage);
	inv_sevcon_tx_msg(&hs_msg);
}

void inv_sevcon_tx_msg(sevcon_hs_t *hs_msg) {
	can_msg_t msg;
	msg.ide = CAN_ID_EXT;
	msg.id = hs_msg->id;
	msg.dlc = sizeof(hs_msg->data);
	memcpy(msg.data, hs_msg->data, msg.dlc);

	can_tx_enqueue(&msg, CAN_SRC_CTRL);
}

#endif
