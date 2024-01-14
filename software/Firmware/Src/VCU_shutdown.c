/*
 * VCU_shutdown.c
 *
 *  Created on: 22 Nov. 2021
 *      Author: Calvin
 *
 *  This file contains functionality for monitoring the shutdown status of the car
 */

#include "VCU_shutdown.h"
#include "can.h"
#include "ads8668.h"

#include <CAN_VCU.h>
#include <CAN_SHDN.h>

ms_timer_t timer_shutdown;
bool shutdown_state_prev;
bool shutdown_state;

bool shutdown_segments[16];

void setup_shutdown_timer() {
	timer_shutdown = timer_init(1, true, shutdown_timer_cb);

	shutdown_state = false;
	shutdown_state_prev = false;

	// start timer
	timer_start(&timer_shutdown);

	// setup ADC channels
	for (int i = 2; i < 6; i++) {
		ADS8668_SetRange(i, ADS8668_RANGE_10V24);
		HAL_Delay(1);
	}
}

void shutdown_timer_cb(void *args) {
	CAN_TxHeaderTypeDef header = { .IDE = CAN_ID_EXT, .RTR = CAN_RTR_DATA,
			.TransmitGlobalTime = DISABLE, };

	// read current state of overall shutdown line
	uint16_t shutdown_read = ADS8668_ReadChannel(ADS8668_MAN_CH_6);

	// 12v voltage divide to 5v, so greater than 3v3
	shutdown_state = ((shutdown_read * 1.25) > 2000);

	// if shutdown changed, send message
	if (shutdown_state != shutdown_state_prev) {
		// just changed state

		// shutdown tripped, so send message
		if (!shutdown_state) {
			SHDN_ShutdownTriggered_t msg = Compose_SHDN_ShutdownTriggered();

			header.ExtId = msg.id;
			header.DLC = 0;

			HAL_CAN_AddTxMessage(&hcan1, &header, NULL, &txMailbox_CAN1);
		}

		shutdown_state_prev = shutdown_state;
	}

	uint16_t shdn_reading_raw[4];
	float shdn_reading[4];

	// read state of 4 ADC lines from shutdown board
	shdn_reading_raw[0] = ADS8668_ReadChannel(ADS8668_MAN_CH_4);
	shdn_reading_raw[1] = ADS8668_ReadChannel(ADS8668_MAN_CH_5);
	shdn_reading_raw[2] = ADS8668_ReadChannel(ADS8668_MAN_CH_3);
	shdn_reading_raw[3] = ADS8668_ReadChannel(ADS8668_MAN_CH_2);

	// convert all readings to correct voltage level
	for (int i = 0; i < 4; i++) {
		shdn_reading[i] = shdn_reading_raw[i] * 0.625f;
	}

	uint16_t thresholds[15] = { 72, 224, 377, 560, 743, 895, 1048, 1274, 1500,
			1653, 1805, 1989, 2172, 2324, 2477 };

	uint8_t shdn_segs[4] = { 0, 0, 0, 0 };

	if (shdn_reading[3] > 1496) {
		shdn_segs[3] = 0;
	} else if (shdn_reading[3] > 1296) {
		shdn_segs[3] = 4;
	} else if (shdn_reading[3] > 1044) {
		shdn_segs[3] = 2;
	} else {
		shdn_segs[3] = 6;
	}

	for (int i = 0; i < 4; i++) {
		/*
		 for (int j = 14; j >= 0; j--) {
		 if (shdn_reading[i] > thresholds[j]) {
		 shdn_segs[i] = j + 1;
		 break;
		 }
		 }
		 */

		for (int j = 0; j < 4; j++) {
			shutdown_segments[i * 4 + j] = ((shdn_segs[i] >> j) & 1) == 1;
		}
	}

	VCU_ShutdownStatus_t segmentsMsg = Compose_VCU_ShutdownStatus(
			shdn_segs[0], shdn_segs[1], shdn_segs[2], shdn_segs[3], shutdown_state);
	header.ExtId = segmentsMsg.id;
	header.DLC = sizeof(segmentsMsg.data);

	HAL_CAN_AddTxMessage(&hcan1, &header, segmentsMsg.data, &txMailbox_CAN1);

}
