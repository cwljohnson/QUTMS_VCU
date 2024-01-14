/*
 * s_dashASB.c
 *
 *  Created on: 1 Dec. 2022
 *      Author: Calvin
 */

#include "s_dashASB.h"
#include "p_isrc.h"

void dashASB_setup() {
	// ADC_CH_DASH_ASB
	isrc_SetCurrentScale(ADC_CH_DASH_ASB, MAX5548_SCALE_5);
	isrc_SetCurrentValue(ADC_CH_DASH_ASB, 200);
	isrc_SetCurrentEnabled(ADC_CH_DASH_ASB, false);
}

void dashASB_setState(bool value) {
	isrc_SetCurrentEnabled(ADC_CH_DASH_ASB, value);
}
