/*
 * comms.c
 *
 *  Created on: 22 May 2023
 *      Author: Calvin
 */

#include <stdint.h>

#include "comms.h"
#include "can_rtos.h"
#include "sensor.h"

#include "heartbeat.h"

#include "inv_sevcon.h"
#include "inv_vesc.h"
#include "main.h"

void comms_tx_timer() {
	static uint16_t counter = 0;

	uint16_t phase_1hz = (counter % TX_TIMER_1HZ_COUNT);
	uint16_t phase_10hz = (counter % TX_TIMER_10HZ_COUNT);
	uint16_t phase_50hz = (counter % TX_TIMER_50HZ_COUNT);
	uint16_t phase_100hz = (counter % TX_TIMER_100HZ_COUNT);


	// in every 1s (eg 1000ms block)
	// first 10ms reserved for heartbeats ->

	if (phase_10hz == VCU_CURRENT_ID) {
		heartbeat_transmit();
	}

#if VCU_CURRENT_ID == VCU_ID_CTRL

	if (phase_50hz == CAN_PHASE_INV) {
#if QEV4 == 1
		inv_sevcon_tx();
#endif

#if QEV3 == 1
		inv_vesc_tx();
#endif
	}



#endif


	// check sensor transmission
	sensor_comms_tx(phase_1hz, phase_10hz, phase_100hz);


	// update counter,
	counter = (counter + 1) % 1000;
}
