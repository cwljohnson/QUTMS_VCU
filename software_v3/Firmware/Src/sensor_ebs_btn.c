/*
 * sensor_ebs_btn.c
 *
 *  Created on: Dec 15, 2023
 *      Author: Calvin
 */

#include "sensor_ebs_btn.h"

#include "heartbeat.h"
#include "task_spi.h"
#include "p_12vSW.h"

#if VCU_CURRENT_ID == VCU_ID_EBS_BTN

void sensor_setup_ebs_btn(void) {

}

void sensor_update_ebs_btn(sensor_data_t *sensor_data) {
	uint16_t btn_voltage = sensor_data->adc_filtered[SENSOR_IDX_P_EBS_BTN]
			* 1.25;

	bool btn_pressed = btn_voltage > 2700;

	VCU_heartbeatState.otherFlags.ebs_btn._VCU_Flags_EBS_BTN.BTN_PRESSED =
			btn_pressed ? 1 : 0;
}

void sensor_ebs_btn_update_outputs(state_machine_t *state_machine) {
	if (heartbeat_boards.EBS_CTRL_hbState.stateID
			== EBS_CTRL_STATE_BRAKE_TEST) {
		// LED ON
		SW_setState(SW_IDX_P_EBS_BTN, true);
	}
	else if (heartbeat_boards.VCU_CTRL_hbState.stateID == VCU_STATE_DVL_IDLE) {
		// flash LED with 500ms period
		if ((state_machine->current_ticks % 500) < 250) {
			SW_setState(SW_IDX_P_EBS_BTN, true);

		}
		else {
			SW_setState(SW_IDX_P_EBS_BTN, false);

		}
	}
	else {
		// LED off
		SW_setState(SW_IDX_P_EBS_BTN, false);
	}
}

#endif
