/*
 * EBS_ADC.c
 *
 *  Created on: Dec 3, 2022
 *      Author: Calvin
 */

#include "EBS_ADC.h"
#include "ads8668.h"
#include "heartbeat.h"

ms_timer_t timer_ebs_adc_check;

void ebs_adc_check_timer_cb(void *args) {
	EBS_ADC_GetDet24V();
	EBS_ADC_GetDetEBSPwr();
	EBS_ADC_GetPChrgBtn();
}

void EBS_ADC_Init() {
	// setup current DACS

	ADS8668_SetRange(ADC_CH_CTRL_EBS, ADS8668_RANGE_10V24);
	ADS8668_SetRange(ADC_CH_CTRL_SHDN, ADS8668_RANGE_10V24);

	timer_ebs_adc_check = timer_init(10, true, ebs_adc_check_timer_cb);
	timer_start(&timer_ebs_adc_check);
}

bool EBS_ADC_GetPChrgBtn() {
	double btnState = adc_readings[ADC_CH_EBS_BTN];

	bool result = btnState > 2600;

	VCU_heartbeatState.otherFlags.ebs_adc._VCU_Flags_EBS_ADC.DET_BTN = result ? 1 : 0;

	return result;
}

// Detects if the EBS is being supplied 24V
// Returns true or false depending if 24V is supplied
bool EBS_ADC_GetDet24V() {
	double DET24V = adc_readings[0];

	bool result = (DET24V >= 4000);

	VCU_heartbeatState.otherFlags.ebs_adc._VCU_Flags_EBS_ADC.DET_24V = result ? 1 : 0;

	return result;
}

// Detects if the EBS is being supplied with PWR
// Returns true or false depending if PWR is supplied
bool EBS_ADC_GetDetEBSPwr() {
	double EBSPOWER = adc_readings[1];

	bool result = (EBSPOWER >= 4000) ;

	VCU_heartbeatState.otherFlags.ebs_adc._VCU_Flags_EBS_ADC.DET_PWR_EBS = result ? 1 : 0;

	return result;

}
