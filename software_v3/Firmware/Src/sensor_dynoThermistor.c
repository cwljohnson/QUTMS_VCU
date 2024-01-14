/*
 * sensor_dynoThermistor.c
 *
 *  Created on: 15 May 2023
 *      Author: Calvin
 */

#include "sensor_dynoThermistor.h"
#include "task_spi.h"
#include "can_rtos.h"

void sensor_update_dynoThermistor() {
	adc_mutex_acquire();

	// take ADC samples
	uint16_t dynoThermADC0 = adc_sample(SENSOR_IDX_DYNOTHERM_0);
	uint16_t dynoThermADC1 = adc_sample(SENSOR_IDX_DYNOTHERM_1);

	adc_mutex_release();

	// make CAN packet
	can_msg_t msg;
	msg.id = 0x1040;
	msg.dlc = 8;
	msg.ide = CAN_ID_EXT;

	for (uint8_t i = 0; i < 2; i++) {
		msg.data[i] = (dynoThermADC0 >> (i*8)) & 0xFF;
		msg.data[2+i] = (dynoThermADC1 >> (i*8)) & 0xFF;
	}

	// transmit CAN
	can_tx_enqueue(&msg, CAN_SRC_SENSOR);
}
