/*
 * MPXV7002.c
 *
 *  Created on: Oct 12, 2021
 *      Author: Calvin
 */

#include "MPXV7002.h"
#include "ads8668.h"
#include "can.h"

#include <CAN_VCU.h>

ms_timer_t timer_pressure;

float MPX_pressureTF(uint16_t voltage) {
	// VCU has VS = 5.02V

	float kpa = ((voltage / 5.02f) - 0.5) / 0.2f;
	return kpa;
}

void setup_pressure() {
	timer_pressure = timer_init(50, true, pressure_timer_cb);

	// start timer
		timer_start(&timer_pressure);
}

void pressure_timer_cb(void *args) {
	CAN_TxHeaderTypeDef header = { .IDE = CAN_ID_EXT, .RTR = CAN_RTR_DATA,
			.TransmitGlobalTime = DISABLE, };

	uint16_t pressure_raw = ADS8668_ReadChannel(ADS8668_MAN_CH_5);
	pressure_raw = pressure_raw * 1.25;

	float pressure = MPX_pressureTF(pressure_raw);

	int16_t pressure_int = (int16_t) (pressure * 10);

	VCU_AirPressure_t msg = Compose_VCU_AirPressure(VCU_ID, pressure_raw, pressure_int);

	header.ExtId = msg.id;
	header.DLC = sizeof(msg.data);

	HAL_CAN_AddTxMessage(&hcan1, &header, msg.data, &txMailbox_CAN1);
}
