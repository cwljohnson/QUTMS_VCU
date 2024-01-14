/*
 * state_machine_states.h
 *
 *  Created on: 6 Sep. 2023
 *      Author: Calvin
 */

#ifndef INC_STATE_MACHINE_STATES_H_
#define INC_STATE_MACHINE_STATES_H_

#include "state_machine.h"

VCU_STATE state_start(state_machine_t *state_machine);
VCU_STATE state_peripheral_init(state_machine_t *state_machine);
VCU_STATE state_sensor_init(state_machine_t *state_machine);
VCU_STATE state_board_check(state_machine_t *state_machine);

// DASH
VCU_STATE state_dash(state_machine_t *state_machine);

// EBS BTN
VCU_STATE state_ebs_btn(state_machine_t *state_machine);

// COOL L/R
VCU_STATE state_cool(state_machine_t *state_machine);

// ACCU
VCU_STATE state_accu(state_machine_t *state_machine);

// SENSOR
VCU_STATE state_sensor(state_machine_t *state_machine);

// CTRL
VCU_STATE state_bmu_check(state_machine_t *state_machine);
VCU_STATE state_idle(state_machine_t *state_machine);
VCU_STATE state_precharge_request(state_machine_t *state_machine);
VCU_STATE state_precharge(state_machine_t *state_machine);
VCU_STATE state_inverter_check(state_machine_t *state_machine);
VCU_STATE state_inverter_energize(state_machine_t *state_machine);
VCU_STATE state_rtd_rdy(state_machine_t *state_machine);
VCU_STATE state_rtd_btn(state_machine_t *state_machine);
VCU_STATE state_driving(state_machine_t *state_machine);
VCU_STATE state_ts_error(state_machine_t *state_machine);

// DVL
VCU_STATE state_dvl_ebs_check(state_machine_t *state_machine);
VCU_STATE state_dvl_rqst_mission(state_machine_t *state_machine);
VCU_STATE state_dvl_idle(state_machine_t *state_machine);
VCU_STATE state_dvl_precharge_request(state_machine_t *state_machine);
VCU_STATE state_dvl_precharge(state_machine_t *state_machine);
VCU_STATE state_dvl_inverter_check(state_machine_t *state_machine);
VCU_STATE state_dvl_rtd(state_machine_t *state_machine);
VCU_STATE state_dvl_driving(state_machine_t *state_machine);
VCU_STATE state_dvl_emergency(state_machine_t *state_machine);

// all
VCU_STATE state_shutdown(state_machine_t *state_machine);
VCU_STATE state_error(state_machine_t *state_machine);

#endif /* INC_STATE_MACHINE_STATES_H_ */
