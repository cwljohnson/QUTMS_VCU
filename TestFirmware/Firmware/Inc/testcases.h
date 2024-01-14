/*
 * testcases.h
 *
 *  Created on: 25 Sep. 2022
 *      Author: Shaq Kuba
 */

#ifndef INC_TESTCASES_H_
#define INC_TESTCASES_H_

#include "7Seg.h"
#include "stm32f2xx_hal.h"
#include "can.h"
#include "12vSW.h"
#include "usbd_cdc.h"
#include "usbd_cdc_if.h"


extern volatile uint8_t adc_num;

enum TestCases{
	CAN1_TX_RX = 0x00,
	CAN2_TX_RX,
	SW0_TOGGLE_12V,
	SW1_TOGGLE_12V,
	SW0_FREQ,
	SW1_FREQ,
	ADC_SAMPLING,
	USB_C_INPUT,
	USB_C_OUTPUT,
	IMU_READING,
	SEG7_LOOP
};

// Function pre-declaration

void run_tests();

void can1_tx_rx( void );
void can2_tx_rx( void );
void USB_output( void );
void seg_7_loop( void );
void SW12v_test( void );

void test_error( void );

void dac_test( void );
void adc_dac_test();

#endif /* INC_TESTCASES_H_ */
