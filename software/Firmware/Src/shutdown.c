/*
 * shutdown.c
 *
 *  Created on: Feb 9, 2022
 *      Author: Calvin
 */


#include "shutdown.h"

#include <CAN_VCU.h>

#include "states.h"
#include "can.h"
#include "heartbeat.h"
#include "RTD.h"


#if (VCU_CURRENT_ID == VCU_ID_CTRL) || (VCU_CURRENT_ID == VCU_ID_EBS)

state_t state_shutdown = { &state_shutdown_enter, &state_shutdown_body, VCU_STATE_SHUTDOWN };

bool shutdown_status = false;

void state_shutdown_enter(fsm_t *fsm) {
#if (VCU_CURRENT_ID == VCU_ID_CTRL)
	// make sure rtd light is off
	rtd_timer_off();
#endif
}

void state_shutdown_body(fsm_t *fsm) {
	CAN_MSG_Generic_t msg;

	// we're in shutdown, so make sure status is bad
	shutdown_status = false;

	while (queue_next(&queue_CAN, &msg)) {
		// check for heartbeats
		if (!check_heartbeat_msg(&msg)) {
			if ((msg.ID & ~0xF) == VCU_ShutdownStatus_ID) {
				uint8_t line;
				bool status;
				Parse_VCU_ShutdownStatus(msg.data, &line, &line, &line, &line, &status);

				// if shutdown is good, then this will kick out of shutdown
				shutdown_status = status;
			}
		}
	}

#if VCU_CURRENT_ID == VCU_ID_CTRL
	if (!check_bad_heartbeat()) {
		// board has dropped out, go to error state

		fsm_changeState(fsm, &state_error, "Board died");
		return;
	}
#endif

	if (shutdown_status) {
		// shutdown is good now, go back to board check
		fsm_changeState(fsm, &state_boardCheck, "Shutdown fixed");
		return;
	}
}

bool check_shutdown_msg(CAN_MSG_Generic_t *msg, bool *shdn_status) {
	bool has_msg = false;

	if ((msg->ID & ~0xF) == VCU_ShutdownStatus_ID) {
		has_msg = true;

		uint8_t line;
		bool status;
		Parse_VCU_ShutdownStatus(msg->data, &line, &line, &line, &line, &status);

		// only update value if we get a shutdown status
		*shdn_status = status;
	}

	else if (msg->ID == SHDN_ShutdownTriggered_ID) {
		has_msg = true;

		// shutdown triggered
		*shdn_status = false;
	}

	return has_msg;
}

#endif
