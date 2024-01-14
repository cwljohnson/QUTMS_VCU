/*
 * states.h
 *
 *  Created on: Jan 18, 2022
 *      Author: Calvin
 */

#ifndef INC_STATES_H_
#define INC_STATES_H_

#include "main.h"

#include <FSM.h>
#include <CAN_VCU.h>

void state_start_enter(fsm_t *fsm);
void state_start_body(fsm_t *fsm);
extern state_t state_start;

void state_pInit_enter(fsm_t *fsm);
void state_pInit_body(fsm_t *fsm);
extern state_t state_pInit;

void state_sInit_enter(fsm_t *fsm);
void state_sInit_body(fsm_t *fsm);
extern state_t state_sInit;

#if VCU_CURRENT_ID == VCU_ID_CTRL

void state_boardCheck_enter(fsm_t *fsm);
void state_boardCheck_body(fsm_t *fsm);
extern state_t state_boardCheck;

void state_checkBMU_enter(fsm_t *fsm);
void state_checkBMU_body(fsm_t *fsm);
extern state_t state_checkBMU;

void state_idle_enter(fsm_t *fsm);
void state_idle_body(fsm_t *fsm);
extern state_t state_idle;

void state_request_pchrg_enter(fsm_t *fsm);
void state_request_pchrg_body(fsm_t *fsm);
extern state_t state_request_pchrg;

void state_precharge_enter(fsm_t *fsm);
void state_precharge_body(fsm_t *fsm);
extern state_t state_precharge;

void state_checkInverter_enter(fsm_t *fsm);
void state_checkInverter_body(fsm_t *fsm);
extern state_t state_checkInverter;

void state_rtdReady_enter(fsm_t *fsm);
void state_rtdReady_body(fsm_t *fsm);
extern state_t state_rtdReady;

void state_rtdButton_enter(fsm_t *fsm);
void state_rtdButton_body(fsm_t *fsm);
extern state_t state_rtdButton;

void state_driving_enter(fsm_t *fsm);
void state_driving_body(fsm_t *fsm);
extern state_t state_driving;

void state_shutdown_enter(fsm_t *fsm);
void state_shutdown_body(fsm_t *fsm);
extern state_t state_shutdown;

void state_tsError_enter(fsm_t *fsm);
void state_tsError_body(fsm_t *fsm);
extern state_t state_tsError;

#if DRIVERLESS_CTRL == 1

void state_DVL_EBS_check_enter(fsm_t *fsm);
void state_DVL_EBS_check_body(fsm_t *fsm);
extern state_t state_DVL_EBS_check;

void state_DVL_RQST_mission_enter(fsm_t *fsm);
void state_DVL_RQST_mission_body(fsm_t *fsm);
extern state_t state_DVL_RQST_mission;

void state_DVL_idle_enter(fsm_t *fsm);
void state_DVL_idle_body(fsm_t *fsm);
extern state_t state_DVL_idle;

void state_DVL_request_pchrg_enter(fsm_t *fsm);
void state_DVL_request_pchrg_body(fsm_t *fsm);
extern state_t state_DVL_request_pchrg;

void state_DVL_precharge_enter(fsm_t *fsm);
void state_DVL_precharge_body(fsm_t *fsm);
extern state_t state_DVL_precharge;

void state_DVL_checkInverter_enter(fsm_t *fsm);
void state_DVL_checkInverter_body(fsm_t *fsm);
extern state_t state_DVL_checkInverter;

void state_DVL_rtd_enter(fsm_t *fsm);
void state_DVL_rtd_body(fsm_t *fsm);
extern state_t state_DVL_rtd;

void state_DVL_driving_enter(fsm_t *fsm);
void state_DVL_driving_body(fsm_t *fsm);
extern state_t state_DVL_driving;

void state_DVL_emergency_enter(fsm_t *fsm);
void state_DVL_emergency_body(fsm_t *fsm);
extern state_t state_DVL_emergency;

#endif

#elif VCU_CURRENT_ID == VCU_ID_SHDN

void state_shdn_enter(fsm_t *fsm);
void state_shdn_body(fsm_t *fsm);
extern state_t state_shdn;

#elif VCU_CURRENT_ID == VCU_ID_DASH

void state_boardCheck_enter(fsm_t *fsm);
void state_boardCheck_body(fsm_t *fsm);
extern state_t state_boardCheck;

void state_dash_enter(fsm_t *fsm);
void state_dash_body(fsm_t *fsm);
extern state_t state_dash;

#elif VCU_CURRENT_ID == VCU_ID_EBS

void state_boardCheck_enter(fsm_t *fsm);
void state_boardCheck_body(fsm_t *fsm);
extern state_t state_boardCheck;

void state_ebs_pwr_enter(fsm_t *fsm);
void state_ebs_pwr_body(fsm_t *fsm);
extern state_t state_ebs_pwr;

void state_ebs_check_asms_enter(fsm_t *fsm);
void state_ebs_check_asms_body(fsm_t *fsm);
extern state_t state_ebs_check_asms;

void state_ebs_check_pressure_enter(fsm_t *fsm);
void state_ebs_check_pressure_body(fsm_t *fsm);
extern state_t state_ebs_check_pressure;

void state_ebs_check_pressure_btn_enter(fsm_t *fsm);
void state_ebs_check_pressure_btn_body(fsm_t *fsm);
extern state_t state_ebs_check_pressure_btn;

void state_ebs_check_pressure_low_enter(fsm_t *fsm);
void state_ebs_check_pressure_low_body(fsm_t *fsm);
extern state_t state_ebs_check_pressure_low;

void state_ebs_check_pressure_high_enter(fsm_t *fsm);
void state_ebs_check_pressure_high_body(fsm_t *fsm);
extern state_t state_ebs_check_pressure_high;

void state_ebs_ctrl_ack_enter(fsm_t *fsm);
void state_ebs_ctrl_ack_body(fsm_t *fsm);
extern state_t state_ebs_ctrl_ack;

void state_ebs_idle_enter(fsm_t *fsm);
void state_ebs_idle_body(fsm_t *fsm);
extern state_t state_ebs_idle;

void state_ebs_pchrg_pressed_enter(fsm_t *fsm);
void state_ebs_pchrg_pressed_body(fsm_t *fsm);
extern state_t state_ebs_pchrg_pressed;

void state_ebs_check_ts_enter(fsm_t *fsm);
void state_ebs_check_ts_body(fsm_t *fsm);
extern state_t state_ebs_check_ts;

void state_ebs_check_compute_enter(fsm_t *fsm);
void state_ebs_check_compute_body(fsm_t *fsm);
extern state_t state_ebs_check_compute;

void state_ebs_ready_enter(fsm_t *fsm);
void state_ebs_ready_body(fsm_t *fsm);
extern state_t state_ebs_ready;

void state_ebs_release_brake_enter(fsm_t *fsm);
void state_ebs_release_brake_body(fsm_t *fsm);
extern state_t state_ebs_release_brake;

void state_ebs_drive_enter(fsm_t *fsm);
void state_ebs_drive_body(fsm_t *fsm);
extern state_t state_ebs_drive;

void state_ebs_braking_enter(fsm_t *fsm);
void state_ebs_braking_body(fsm_t *fsm);
extern state_t state_ebs_braking;

void state_shutdown_enter(fsm_t *fsm);
void state_shutdown_body(fsm_t *fsm);
extern state_t state_shutdown;

#elif VCU_CURRENT_ID == VCU_ID_ASSI

void state_boardCheck_enter(fsm_t *fsm);
void state_boardCheck_body(fsm_t *fsm);
extern state_t state_boardCheck;

void state_assi_enter(fsm_t *fsm);
void state_assi_body(fsm_t *fsm);
extern state_t state_assi;

#elif VCU_CURRENT_ID == VCU_ID_EBS_ADC

void state_ebs_adc_enter(fsm_t *fsm);
void state_ebs_adc_body(fsm_t *fsm);
extern state_t state_ebs_adc;

#endif

void state_error_enter(fsm_t *fsm);
void state_error_body(fsm_t *fsm);
extern state_t state_error;

#endif /* INC_STATES_H_ */
