/*
 * testcases.c
 *
 *  Created on: 25 Sep. 2022
 *      Author: Shaq Kuba
 */
// Includes
#include "testcases.h"
#include <string.h>
#include "max5548.h"
#include "ads8668.h"
#include "7seg.h"

// Variables
extern message_queue_t queue_CAN1_Rx;
extern message_queue_t queue_CAN2_Rx;
extern volatile uint8_t compare;

// Change this for ADC testing
volatile uint8_t adc_num = 2;

void run_tests() {

	//can1_tx_rx();
	//can2_tx_rx();
	//SW12v_test();
}

// CAN TESTS
void can1_tx_rx(void) {
	for (int i = 0; i < 10; i++) {
		// Send message over CAN1
		CAN_TxHeaderTypeDef TxHeader = { .ExtId = 0x69 + i, .IDE = CAN_ID_EXT,
				.RTR = CAN_RTR_DATA, .DLC = 4, .TransmitGlobalTime = DISABLE, };
		uint8_t Tx_msg_data[4] = { 1 * i, 2 * i, 3 * i, 4 * i };
		send_can_msg(&hcan1, &TxHeader, Tx_msg_data);

		// Read message over CAN2
		CAN_RxHeaderTypeDef RxHeader;
		uint8_t Rx_msg_data[4];
		if (HAL_CAN_GetRxMessage(&hcan2, CAN_RX_FIFO0, &RxHeader, Rx_msg_data)
				== HAL_OK) {
			set_7seg(i, i % 2 == 0);
		} else {
			set_7seg(0xF, 0xF % 2 == 0);
		}
		HAL_Delay(500);
	}
	set_7seg(0xD, 0xD % 2 == 0); // Showing D for done test
}

void can2_tx_rx(void) {
	for (int i = 0; i < 5; i++) {
		// Send message over CAN1
		CAN_TxHeaderTypeDef TxHeader = { .ExtId = 0x69 + i, .IDE = CAN_ID_EXT,
				.RTR = CAN_RTR_DATA, .DLC = 4, .TransmitGlobalTime = DISABLE, };
		uint8_t Tx_msg_data[4] = { 1 * i, 2 * i, 3 * i, 4 * i };
		send_can_msg(&hcan2, &TxHeader, Tx_msg_data);

		// Read message over CAN2
		CAN_RxHeaderTypeDef RxHeader;
		uint8_t Rx_msg_data[4];
		if (HAL_CAN_GetRxMessage(&hcan1, CAN_RX_FIFO0, &RxHeader, Rx_msg_data)
				== HAL_OK) {
			set_7seg(i, i % 2 == 0);
		} else {
			set_7seg(0xF, 0xF % 2 == 0);
		}
		HAL_Delay(500);
	}
	set_7seg(0xD, 0xD % 2 == 0); // Showing D for done test
}

extern message_queue_t queue_CAN1_Rx;
extern message_queue_t queue_CAN2_Rx;

void can_tx_rx_interrupt(message_queue_t CAN_QUEUE) {
	while (1) {
		if (CAN_QUEUE.count < 0xF) {
			set_7seg(CAN_QUEUE.count, CAN_QUEUE.count % 2 == 0);
		}
	}

}

// 12V SW0/SW1 TESTS
void SW12v_test(void) {
	bool up = true;
	while (1) {
		if (compare < 90 && up) {
			compare = compare + 2;
			if (compare == 90) {
				up = false;
			}
		} else {
			compare = compare - 2;
			if (compare == 10) {
				up = true;
			}
		}
		HAL_Delay(500);
	}
}

// ADC SAMPLING TESTS

// USB-C TESTS

void USB_output(void) {
	/*
	 uint8_t* buff = "Testing USB\r\n";

	 //uint8_t buff[4] = {0,1,2,3};

	 while(1)
	 {
	 CDC_Transmit_FS(buff, strlen(buff));
	 HAL_Delay(1000); // 1ms delay
	 }*/

}
// IMU TESTS

// SEG 7  TEST
void seg_7_loop(void) {
	for (int i = 0; i <= 0xF; i++) {
		set_7seg(i, i % 2 == 0);
		HAL_Delay(500);
	}

}

// ERROR
void test_error(void) {
	while (1) {

	}
}

// ADC DAC TEST
void adc_dac_test() {

	setup_ISRC();
	ADS8668_Init();

	for (int i = 0; i < 8; i++) {
		isrc_SetCurrentScale(i, MAX5548_SCALE_2);
		isrc_SetCurrentValue(i, 50);
	}

	uint32_t adc_count = HAL_GetTick();
	uint32_t dac_count = HAL_GetTick();

	bool state = false;

	char buffer[200];
	int num = 0;

	while (1) {
		if ((HAL_GetTick() - adc_count) > 250) {
			adc_count = HAL_GetTick();

			ADS8668_ReadAll();

			num = sprintf(buffer, "ADC: %4i %4i %4i %4i %4i %4i %4i %4i\r\n",
					(int) adc_readings[0], (int) adc_readings[1],
					(int) adc_readings[2], (int) adc_readings[3],
					(int) adc_readings[4], (int) adc_readings[5],
					(int) adc_readings[6], (int) adc_readings[7]);
			CDC_Transmit_FS(buffer, num);
			HAL_Delay(5);
		}

		if ((HAL_GetTick() - dac_count) > 1000) {
			dac_count = HAL_GetTick();

			state = !state;
			for (int i = 0; i < 8; i++) {
				isrc_SetCurrentEnabled(i, state);
			}

			num = sprintf(buffer, "DAC: %i\r\n", state ? 1 : 0);
			CDC_Transmit_FS(buffer, num);
			HAL_Delay(5);
		}
	}
}

// DAC TEST

void dac_test(void) {
	// 2 -

	// 3
	int idx = 2;

	// Initialize
	MAX5548_init(idx);

	uint8_t dac_scales[6] = { MAX5548_SCALE_0, MAX5548_SCALE_1, MAX5548_SCALE_2,
	MAX5548_SCALE_3, MAX5548_SCALE_4, MAX5548_SCALE_5 };

	MAX5548_EnableChannel(idx, MAX5548_CH_A, true);
	MAX5548_EnableChannel(idx, MAX5548_CH_B, true);

	/*while(true) {
	 int dispCount = 0;
	 uint32_t sleepAmount = 10000;

	 for(int j = 0; j < 6; j++){

	 MAX5548_SetChannelScale(idx, MAX5548_CH_A, dac_scales[j]);
	 MAX5548_SetChannelScale(idx, MAX5548_CH_B, dac_scales[j]);

	 MAX5548_SetChannelValue(idx, MAX5548_CH_A, 100);
	 MAX5548_SetChannelValue(idx, MAX5548_CH_B, 100);

	 dispCount++;
	 set_7seg(dispCount, false);

	 HAL_Delay(sleepAmount);

	 MAX5548_SetChannelValue(idx, MAX5548_CH_A, 200);
	 MAX5548_SetChannelValue(idx, MAX5548_CH_B, 200);

	 dispCount++;
	 set_7seg(dispCount, true);

	 HAL_Delay(sleepAmount);
	 }
	 }
	 */

//Test single scale current
	while (true) {
		int dispCount = 0;
		uint32_t sleepAmount = 10000;

		MAX5548_SetChannelScale(idx, MAX5548_CH_A, dac_scales[4]);
		MAX5548_SetChannelScale(idx, MAX5548_CH_B, dac_scales[4]);

		for (int j = 10; j <= 250; j += 10) {

			MAX5548_SetChannelValue(idx, MAX5548_CH_A, j);
			MAX5548_SetChannelValue(idx, MAX5548_CH_B, j);

			dispCount++;
			set_7seg(dispCount, false);

			HAL_Delay(sleepAmount);
		}
	}

}

