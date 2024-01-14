/*
 * s_suspensionTravel.c
 *
 *  Created on: 17 Feb. 2022
 *      Author: Calvin
 */

#include "can.h"
#include "s_suspensionTravel.h"
#include "ads8668.h"
#include "p_adc.h"
#include "p_isrc.h"
#include "heartbeat.h"

int suspension_transmit_count = 0;

ms_timer_t timer_suspension_adc;

void setup_susTravel() {
	// enable filters on the adcs
	ADS8668_FilterEnable(ADC_CH_SUS_LEFT);
	ADS8668_FilterEnable(ADC_CH_SUS_RIGHT);

	// set range
	ADS8668_SetRange(ADC_CH_SUS_LEFT, ADS8668_RANGE_5V12);
	ADS8668_SetRange(ADC_CH_SUS_RIGHT, ADS8668_RANGE_5V12);

	// set current pull ups so we can detect disconnection
/*
	// ~1 mA
	isrc_SetCurrentScale(ADC_CH_SUS_LEFT, MAX5548_SCALE_0);
	isrc_SetCurrentValue(ADC_CH_SUS_LEFT, 180);

	isrc_SetCurrentScale(ADC_CH_SUS_RIGHT, MAX5548_SCALE_0);
	isrc_SetCurrentValue(ADC_CH_SUS_RIGHT, 180);

	isrc_SetCurrentEnabled(ADC_CH_SUS_LEFT, true);
	isrc_SetCurrentEnabled(ADC_CH_SUS_RIGHT, true);
*/
	// every 20ms check values
	timer_suspension_adc = timer_init(20, true, suspension_timer_cb);

	timer_start(&timer_suspension_adc);
}

bool check_susTravel_connected() {
	bool success = true;

	if (ADS8668_GetScaledFiltered(ADC_CH_SUS_LEFT) < ADC_CUTOFF_PULLDOWN ) {
		// TODO: FIX TO MATCH CAR???
		//success = false;

#if VCU_CURRENT_ID == VCU_ID_DASH
		VCU_heartbeatState.otherFlags.dash._VCU_Flags_Dash.S_Sus_L = 1;
#elif VCU_CURRENT_ID == VCU_ID_SHDN
		VCU_heartbeatState.otherFlags.shdn._VCU_Flags_SHDN.S_Sus_L = 1;
#endif
	}
	else {
#if VCU_CURRENT_ID == VCU_ID_DASH
		VCU_heartbeatState.otherFlags.dash._VCU_Flags_Dash.S_Sus_L = 0;
#elif VCU_CURRENT_ID == VCU_ID_SHDN
		VCU_heartbeatState.otherFlags.shdn._VCU_Flags_SHDN.S_Sus_L = 0;
#endif
	}

	if (ADS8668_GetScaledFiltered(ADC_CH_SUS_RIGHT) < ADC_CUTOFF_PULLDOWN) {
		// TODO: FIX TO MATCH CAR???
		//success = false;

#if VCU_CURRENT_ID == VCU_ID_DASH
		VCU_heartbeatState.otherFlags.dash._VCU_Flags_Dash.S_Sus_R = 1;
#elif VCU_CURRENT_ID == VCU_ID_SHDN
		VCU_heartbeatState.otherFlags.shdn._VCU_Flags_SHDN.S_Sus_R = 1;
#endif
	}
	else {
#if VCU_CURRENT_ID == VCU_ID_DASH
		VCU_heartbeatState.otherFlags.dash._VCU_Flags_Dash.S_Sus_R = 0;
#elif VCU_CURRENT_ID == VCU_ID_SHDN
		VCU_heartbeatState.otherFlags.shdn._VCU_Flags_SHDN.S_Sus_R = 0;
#endif
	}

	return success;
}

void suspension_timer_cb(void *args) {
	check_susTravel_connected();

	// this timer runs every 20ms, want to send suspension travel on CAN every 100ms, so only send every 5 calls
	suspension_transmit_count++;

	if (suspension_transmit_count >= 5) {
		suspension_transmit_count = 0;

#if VCU_CURRENT_ID == VCU_ID_DASH
		bool front = true;
#elif VCU_CURRENT_ID == VCU_ID_SHDN
		bool front = false;
#endif

#if (VCU_CURRENT_ID == VCU_ID_DASH) || (VCU_CURRENT_ID == VCU_ID_SHDN)
		// sent to CAN
		VCU_LinearTravel_t msg = Compose_VCU_LinearTravel(VCU_CURRENT_ID, front,
				(uint16_t) ADS8668_GetScaledFiltered(ADC_CH_SUS_LEFT),
				(uint16_t) ADS8668_GetScaledFiltered(ADC_CH_SUS_RIGHT));

		CAN_TxHeaderTypeDef header = { .ExtId = msg.id, .IDE =
		CAN_ID_EXT, .RTR = CAN_RTR_DATA, .DLC = sizeof(msg.data), .TransmitGlobalTime = DISABLE, };

		send_can_msg(&hcan1, &header, msg.data);
#endif
	}
}
