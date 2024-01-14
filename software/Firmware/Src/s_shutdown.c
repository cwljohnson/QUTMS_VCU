/*
 * s_shutdown.c
 *
 *  Created on: 21 Feb. 2022
 *      Author: Calvin J
 */

#include "s_shutdown.h"
#include "ads8668.h"

#include <CAN_VCU.h>
#include <CAN_SHDN.h>

#include "can.h"
#include "heartbeat.h"
#include "p_isrc.h"

ms_timer_t timer_shdn_status;
bool shdn_state_prev;
bool shdn_state;

uint16_t shdn_reading[4];
uint8_t shdn_segs[4];

void setup_shutdown() {
	timer_shdn_status = timer_init(1, true, shdn_status_timer_cb);

	timer_start(&timer_shdn_status);

	shdn_state = false;
	shdn_state_prev = false;

	// disable DACs
	isrc_SetCurrentEnabled(ADC_CH_SHDN_0, false);
	isrc_SetCurrentEnabled(ADC_CH_SHDN_1, false);
	isrc_SetCurrentEnabled(ADC_CH_SHDN_2, false);
	isrc_SetCurrentEnabled(ADC_CH_SHDN_3, false);

	// setup ADC channels
	ADS8668_SetRange(ADC_CH_SHDN_0, ADS8668_RANGE_2V56);
	ADS8668_SetRange(ADC_CH_SHDN_1, ADS8668_RANGE_2V56);
	ADS8668_SetRange(ADC_CH_SHDN_2, ADS8668_RANGE_2V56);
	ADS8668_SetRange(ADC_CH_SHDN_3, ADS8668_RANGE_2V56);
	ADS8668_SetRange(ADC_CH_SHDN_STATUS, ADS8668_RANGE_5V12);
}

void shdn_status_timer_cb(void *args) {
	CAN_TxHeaderTypeDef header = { .IDE = CAN_ID_EXT, .RTR = CAN_RTR_DATA,
				.TransmitGlobalTime = DISABLE, };

	// read current state of overall shutdown line
	float shutdown_read = adc_readings[ADC_CH_SHDN_STATUS];

	// shdn_state == true (1) -> shutdown line is GOOD
	// false (0) -> shutdown line is BAD
	shdn_state = (shutdown_read > 2000);

	// if shutdown changed, send message
	if (shdn_state != shdn_state_prev) {
		// just changed state

		// shutdown tripped, so send message
		if (!shdn_state) {
			SHDN_ShutdownTriggered_t msg = Compose_SHDN_ShutdownTriggered();

			header.ExtId = msg.id;
			header.DLC = 0;

			send_can_msg(&hcan1, &header, NULL);
		}

		shdn_state_prev = shdn_state;
	}

	// heartbeat flags are 1 for bad, so need to flip shdn_state
	VCU_heartbeatState.otherFlags.shdn._VCU_Flags_SHDN.SHDN_Status = shdn_state ? 0 : 1;


	shdn_reading[0] = adc_readings_raw[ADC_CH_SHDN_0];
	shdn_reading[1] = adc_readings_raw[ADC_CH_SHDN_1];
	shdn_reading[2] = adc_readings_raw[ADC_CH_SHDN_2];
	shdn_reading[3] = adc_readings_raw[ADC_CH_SHDN_3];

	for (int i = 0; i < 4; i++) {
		shdn_segs[i] = ((shdn_reading[i] + 40 ) + 0x80) >> 8;
	}

	VCU_ShutdownStatus_t segmentsMsg = Compose_VCU_ShutdownStatus(shdn_segs[0], shdn_segs[1], shdn_segs[2],
			shdn_segs[3], shdn_state);
	header.ExtId = segmentsMsg.id;
	header.DLC = sizeof(segmentsMsg.data);

	send_can_msg(&hcan1, &header, segmentsMsg.data);
}
