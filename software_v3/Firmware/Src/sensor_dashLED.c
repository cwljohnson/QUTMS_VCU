/*
 * sensor_dashLED.c
 *
 *  Created on: Dec 4, 2023
 *      Author: Calvin
 */

#include "sensor_dashLED.h"
#include "heartbeat.h"
#include "task_spi.h"

void sensor_setup_dashLED(void) {
	isrc_mutex_acquire();

	isrc_change_scale(DASH_CH_LED_BSPD, MAX5548_SCALE_5);
	isrc_change_scale(DASH_CH_LED_IMD, MAX5548_SCALE_5);
	isrc_change_scale(DASH_CH_LED_AMS, MAX5548_SCALE_5);
	isrc_change_scale(DASH_CH_LED_PDOC, MAX5548_SCALE_5);

	isrc_change_value(DASH_CH_LED_BSPD, 200);
	isrc_change_value(DASH_CH_LED_IMD, 200);
	isrc_change_value(DASH_CH_LED_AMS, 200);
	isrc_change_value(DASH_CH_LED_PDOC, 200);

	isrc_mutex_release();

	// set all LEDS off initially
	sensor_dashLED_setState(DASH_CH_LED_AMS, false);
	sensor_dashLED_setState(DASH_CH_LED_BSPD, false);
	sensor_dashLED_setState(DASH_CH_LED_IMD, false);
	sensor_dashLED_setState(DASH_CH_LED_PDOC, false);
}

void sensor_dashLED_setState(uint8_t idx, bool on) {
	isrc_mutex_acquire();

	isrc_change_enabled(idx, on);

	isrc_mutex_release();

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
