/*
 * s_dashLED.c
 *
 *  Created on: 21 Feb. 2022
 *      Author: Calvin J
 */

#include "s_dashLED.h"
#include "p_isrc.h"
#include "heartbeat.h"

void setup_dashLED() {
	// by default, all LEDs start on
	// then turn off when good, then they'll latch on if they get tripped
	// till shutdown reset

	// init all LEDS with correct current values
	isrc_SetCurrentScale(DASH_CH_LED_AMS, MAX5548_SCALE_5);
	isrc_SetCurrentScale(DASH_CH_LED_BSPD, MAX5548_SCALE_5);
	isrc_SetCurrentScale(DASH_CH_LED_IMD, MAX5548_SCALE_5);
	isrc_SetCurrentScale(DASH_CH_LED_PDOC, MAX5548_SCALE_5);

	isrc_SetCurrentValue(DASH_CH_LED_AMS, 200);
	isrc_SetCurrentValue(DASH_CH_LED_BSPD, 200);
	isrc_SetCurrentValue(DASH_CH_LED_IMD, 200);
	isrc_SetCurrentValue(DASH_CH_LED_PDOC, 200);

	// set all LEDS off initially
	dashLED_setState(DASH_CH_LED_AMS, false);
	dashLED_setState(DASH_CH_LED_BSPD, false);
	dashLED_setState(DASH_CH_LED_IMD, false);
	dashLED_setState(DASH_CH_LED_PDOC, false);
}

void dashLED_setState(uint8_t idx, bool on) {
	//isrc_SetCurrent(idx, false, on ? 127 : 0);
	isrc_SetCurrentEnabled(idx, on);

	if (idx == DASH_CH_LED_AMS) {
		VCU_heartbeatState.otherFlags.dash._VCU_Flags_Dash.LED_AMS = on ? 1 : 0;
	} else if (idx == DASH_CH_LED_BSPD) {
		VCU_heartbeatState.otherFlags.dash._VCU_Flags_Dash.LED_BSPD = on ? 1 : 0;
	} else if (idx == DASH_CH_LED_IMD) {
		VCU_heartbeatState.otherFlags.dash._VCU_Flags_Dash.LED_IMD = on ? 1 : 0;
	} else if (idx == DASH_CH_LED_PDOC) {
		VCU_heartbeatState.otherFlags.dash._VCU_Flags_Dash.LED_PDOC = on ? 1 : 0;
	}
}
