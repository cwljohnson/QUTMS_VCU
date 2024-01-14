/*
 * state_machine.c
 *
 *  Created on: Sep 5, 2023
 *      Author: Calvin
 */

#include <sensor_rear_fan.h>
#include "state_machine.h"
#include "state_machine_states.h"
#include "usbd_cdc_if.h"
#include "heartbeat.h"
#include <stdio.h>

#include "sensor_rtd.h"
#include "sensor_brakelight.h"
#include "sensor_cooling.h"
#include "sensor_rear_fan.h"
#include "sensor_ebs_btn.h"
#include "inv_sevcon.h"
#include "main.h"

#include "cmsis_os.h"

static char usb_buffer[100];

void state_machine_init(state_machine_t *state_machine) {
	state_machine->state_current = VCU_STATE_START;

	// store shutdown status
	state_machine->shutdown_status.state = true;
	state_machine->shutdown_status.segs[0] = 0;
	state_machine->shutdown_status.segs[1] = 0;
	state_machine->shutdown_status.segs[2] = 0;
	state_machine->shutdown_status.segs[3] = 0;

	state_machine_change_state(state_machine, VCU_STATE_START);
}

VCU_STATE state_machine_update(state_machine_t *state_machine) {
	VCU_STATE new_state = state_machine->state_current;
	state_machine->current_ticks = osKernelGetTickCount()
			- state_machine->enter_ticks;

	switch (new_state) {
	case VCU_STATE_START:
		new_state = state_start(state_machine);
		break;
	case VCU_STATE_PERIPHERAL_INIT:
		new_state = state_peripheral_init(state_machine);
		break;
	case VCU_STATE_SENSOR_INIT:
		new_state = state_sensor_init(state_machine);
		break;
	case VCU_STATE_BOARD_CHECK:
		new_state = state_board_check(state_machine);
		break;
#if VCU_CURRENT_ID == VCU_ID_CTRL
	case VCU_STATE_BMU_CHECK:
		new_state = state_bmu_check(state_machine);
		break;
	case VCU_STATE_IDLE:
		new_state = state_idle(state_machine);
		break;
	case VCU_STATE_PRECHARGE_REQUEST:
		new_state = state_precharge_request(state_machine);
		break;
	case VCU_STATE_PRECHARGE:
		new_state = state_precharge(state_machine);
		break;
	case VCU_STATE_INVERTER_CHECK:
		new_state = state_inverter_check(state_machine);
		break;
#if QEV4 == 1
	case VCU_STATE_INVERTER_ENERGIZE:
		new_state = state_inverter_energize(state_machine);
		break;
#endif
	case VCU_STATE_RTD_RDY:
		new_state = state_rtd_rdy(state_machine);
		break;
	case VCU_STATE_RTD_BTN:
		new_state = state_rtd_btn(state_machine);
		break;
	case VCU_STATE_DRIVING:
		new_state = state_driving(state_machine);
		break;
	case VCU_STATE_TS_ERROR:
		new_state = state_ts_error(state_machine);
		break;
#if QEV3 == 1
#if DRIVERLESS_CTRL == 1
	case VCU_STATE_DVL_EBS_CHECK:
		new_state = state_dvl_ebs_check(state_machine);
		break;
	case VCU_STATE_DVL_RQST_MISSION:
		new_state = state_dvl_rqst_mission(state_machine);
		break;
	case VCU_STATE_DVL_IDLE:
		new_state = state_dvl_idle(state_machine);
		break;
	case VCU_STATE_DVL_PRECHARGE_REQUEST:
		new_state = state_dvl_precharge_request(state_machine);
		break;
	case VCU_STATE_DVL_PRECHARGE:
		new_state = state_dvl_precharge(state_machine);
		break;
	case VCU_STATE_DVL_INVERTER_CHECK:
		new_state = state_dvl_inverter_check(state_machine);
		break;
	case VCU_STATE_DVL_RTD:
		new_state = state_dvl_rtd(state_machine);
		break;
	case VCU_STATE_DVL_DRIVING:
		new_state = state_dvl_driving(state_machine);
		break;
	case VCU_STATE_DVL_EMERGENCY:
		new_state = state_dvl_emergency(state_machine);
		break;
#endif
#endif
#elif (VCU_CURRENT_ID == VCU_ID_SENSOR)
	case VCU_STATE_SENSOR:
		new_state = state_sensor(state_machine);
		break;
#elif (VCU_CURRENT_ID == VCU_ID_COOL_L) || (VCU_CURRENT_ID == VCU_ID_COOL_R)
	case VCU_STATE_COOL:
		new_state = state_cool(state_machine);
		break;
#elif (VCU_CURRENT_ID == VCU_ID_ACCU)
	case VCU_STATE_ACCU:
		new_state = state_accu(state_machine);
		break;
#elif (VCU_CURRENT_ID == VCU_ID_DASH)
	case VCU_STATE_DASH:
		new_state = state_dash(state_machine);
		break;
#endif
	case VCU_STATE_SHUTDOWN:
		new_state = state_shutdown(state_machine);
		break;
	case VCU_STATE_ERROR:
		new_state = state_error(state_machine);
		break;
	default:
		new_state = VCU_STATE_ERROR;
		break;
	}

	if (new_state != state_machine->state_current) {
		(void) state_machine_change_state(state_machine, new_state);
	}
	else {
		state_machine->state_counter++;
	}

	return new_state;
}

VCU_STATE state_machine_change_state(state_machine_t *state_machine,
		VCU_STATE new_state) {
	// changed state
	uint16_t num = sprintf(usb_buffer, "STATE CHANGE: (0x%02X) -> (0x%02X)\r\n",
			state_machine->state_current, new_state);
	CDC_Transmit_FS((uint8_t*) usb_buffer, num);
	state_machine->state_current = new_state;
	state_machine->state_counter = 0U;

	// update heartbeat object
	VCU_heartbeatState.stateID = state_machine->state_current;

	state_machine->enter_ticks = osKernelGetTickCount();
	state_machine->current_ticks = 0;

	return new_state;
}

VCU_STATE state_machine_handle_CAN(state_machine_t *state_machine,
		can_msg_t *msg) {
	VCU_STATE new_state = state_machine->state_current;

	// process CAN messages

	bool result = false;

#if (VCU_CURRENT_ID != VCU_ID_ACCU) && (VCU_CURRENT_ID != VCU_ID_SHDN)
	if (!result) {
		if (msg->id == VCU_ShutdownStatus_ID) {
			result = true;

			Parse_VCU_ShutdownStatus(msg->data,
					&state_machine->shutdown_status.segs[0],
					&state_machine->shutdown_status.segs[1],
					&state_machine->shutdown_status.segs[2],
					&state_machine->shutdown_status.segs[3],
					&state_machine->shutdown_status.state);

#if (VCU_CURRENT_ID != VCU_ID_DASH)
			if (state_machine->shutdown_status.state) {
				// shutdown good
				if (state_machine->state_current == VCU_STATE_SHUTDOWN) {
					// go back out
					new_state = VCU_STATE_BOARD_CHECK;
					return new_state;
				}
			}
			else {
				// shutdown bad
				if ((state_machine->state_current != VCU_STATE_SHUTDOWN)
						&& (state_machine->state_current != VCU_STATE_ERROR)
								& (state_machine->state_current
										!= VCU_STATE_TS_ERROR)
						&& (state_machine->state_current
								!= VCU_STATE_DVL_EMERGENCY)) {
					if (state_machine->state_current > VCU_STATE_BOARD_CHECK) {
						// go to shutdown
						new_state = VCU_STATE_SHUTDOWN;
						return new_state;
					}
				}
			}
#endif
		}
	}
#endif

	return new_state;
}

void state_machine_set_outputs(state_machine_t *state_machine) {
#if VCU_CURRENT_ID == VCU_ID_CTRL
	sensor_rtd_update_outputs(state_machine);
#endif

#if (VCU_CURRENT_ID == VCU_ID_ACCU) || (VCU_CURRENT_ID == VCU_ID_DASH)
	sensor_brakelight_update_outputs(state_machine);
#endif

#if (VCU_CURRENT_ID == VCU_ID_ACCU)
	sensor_rear_fan_update_outputs(state_machine);
#endif

#if (VCU_CURRENT_ID == VCU_ID_COOL_L) || (VCU_CURRENT_ID == VCU_ID_COOL_R)
	sensor_cooling_update_outputs(state_machine);
#endif

#if VCU_CURRENT_ID == VCU_ID_EBS_BTN
	sensor_ebs_btn_update_outputs(state_machine);
#endif
}

