/*
 * EBS.h
 *
 *  Created on: Jul 6, 2022
 *      Author: Alex Pearl
 */

#ifndef INC_EBS_H_
#define INC_EBS_H_

#include <stdbool.h>
#include <Timer.h>

#define DAC_IDX_EBS 2

#define ADC_CH_CTRL_EBS 7
#define ADC_CH_CTRL_SHDN 6


#define EBS_PRESSURE_THRESH_HIGH 1700
#define EBS_PRESSURE_THRESH_LOW 800

// 500ms
#define EBS_BTN_TOGGLE 500
// 2000ms
#define EBS_BTN_HOLD_TIME 2000

void EBS_SetPWR(bool state);


bool EBS_GetPChrgBtn();
bool EBS_GetDet24V();
bool EBS_GetDetEBSPwr();


void EBS_SetPChrgBtnLED(bool value);

void EBS_SetCtrlEBS(bool value);

void EBS_SetCtrlSHDN(bool value);

void EBS_Init();

extern ms_timer_t timer_ebs_check;
void ebs_check_timer_cb(void *args);


#endif /* INC_EBS_H_ */
