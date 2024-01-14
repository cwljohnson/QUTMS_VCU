/*
 * EBS_ADC.h
 *
 *  Created on: Dec 3, 2022
 *      Author: Calvin Johnson
 */

#ifndef INC_EBS_ADC_H_
#define INC_EBS_ADC_H_

#include <stdbool.h>
#include <Timer.h>

#define ADC_CH_CTRL_EBS 7
#define ADC_CH_CTRL_SHDN 6
#define ADC_CH_EBS_BTN 3

extern ms_timer_t timer_ebs_adc_check;

void ebs_adc_check_timer_cb(void *args);

void EBS_ADC_Init();

bool EBS_ADC_GetPChrgBtn();

// Detects if the EBS is being supplied 24V
// Returns true or false depending if 24V is supplied
bool EBS_ADC_GetDet24V();

// Detects if the EBS is being supplied with PWR
// Returns true or false depending if PWR is supplied
bool EBS_ADC_GetDetEBSPwr();

#endif /* INC_EBS_ADC_H_ */
