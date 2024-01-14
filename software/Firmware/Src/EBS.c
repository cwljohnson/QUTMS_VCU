/*
 * EBS.c
 *
 *  Created on: Jul 6, 2022
 *      Author: Alex Pearl
 */
#include "ads8668.h"
#include "p_isrc.h"
#include "12vSW.h"
#include "spi.h"
#include "EBS.h"
#include "heartbeat.h"

/*
ms_timer_t timer_ebs_check;
void ebs_check_timer_cb(void *args) {
	EBS_GetDet24V();
	EBS_GetDetEBSPwr();
}
*/

void EBS_Init() {
	// setup current DACS
	ADS8668_SetRange(ADC_CH_CTRL_EBS, ADS8668_RANGE_10V24);
	ADS8668_SetRange(ADC_CH_CTRL_SHDN, ADS8668_RANGE_10V24);

	isrc_SetCurrentScale(ADC_CH_CTRL_EBS, MAX5548_SCALE_2);
	isrc_SetCurrentScale(ADC_CH_CTRL_SHDN, MAX5548_SCALE_2);

	isrc_SetCurrentValue(ADC_CH_CTRL_EBS, 0);
	isrc_SetCurrentValue(ADC_CH_CTRL_SHDN, 0);

	isrc_SetCurrentEnabled(ADC_CH_CTRL_EBS, true);
	isrc_SetCurrentEnabled(ADC_CH_CTRL_SHDN, true);
/*
	timer_ebs_check = timer_init(10, true, ebs_check_timer_cb);
	timer_start(&timer_ebs_check);
	*/
}

// Set 12V input true or false to set state of GPIO pins
void EBS_SetPWR(bool state) {
	SW_setState(0, state);

	VCU_heartbeatState.otherFlags.ebs._VCU_Flags_EBS.CTRL_PWR = state ? 1 : 0;
}

void EBS_SetPChrgBtnLED(bool value) {
	SW_setState(1, value);
}

// Sets EBS Control
// Inout true or fale to set channel value to either 50 or 0
void EBS_SetCtrlEBS(bool value) {
	isrc_SetCurrentValue(ADC_CH_CTRL_EBS, value ? 100 : 10);

	VCU_heartbeatState.otherFlags.ebs._VCU_Flags_EBS.CTRL_EBS = value ? 1 : 0;
}

// Sets Ctrl Shut Down
// Input true or false to set Channel Value to either 50 or 0
void EBS_SetCtrlSHDN(bool value) {
	isrc_SetCurrentValue(ADC_CH_CTRL_SHDN, value ? 100 : 10);

	VCU_heartbeatState.otherFlags.ebs._VCU_Flags_EBS.CTRL_SHDN = value ? 1 : 0;

}

bool EBS_GetPChrgBtn() {
	return VCU_heartbeatState.otherFlags.ebs._VCU_Flags_EBS.DET_BTN == 1;
}

// Detects if the EBS is being supplied 24V
// Returns true or false depending if 24V is supplied
bool EBS_GetDet24V() {
	return VCU_heartbeatState.otherFlags.ebs._VCU_Flags_EBS.DET_24V == 1;
}

// Detects if the EBS is being supplied with PWR
// Returns true or false depending if PWR is supplied
bool EBS_GetDetEBSPwr() {
	return VCU_heartbeatState.otherFlags.ebs._VCU_Flags_EBS.DET_PWR_EBS == 1;
}
