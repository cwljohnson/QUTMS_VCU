/*
 * 12vSW.c
 *
 *  Created on: Oct 10, 2021
 *      Author: Calvin
 */


#include "12vSW.h"
#include "main.h"

volatile bool SW1state = false;
volatile uint8_t counter = 0;
volatile uint8_t compare = 20;

#define TEST 1

void SW_setState(uint8_t idx, bool enable) {

	if (idx == 0) {
		HAL_GPIO_WritePin(PROFET_IN0_GPIO_Port, PROFET_IN0_Pin, enable ? GPIO_PIN_SET : GPIO_PIN_RESET);
	} else if (idx == 1) {
		HAL_GPIO_WritePin(PROFET_IN1_GPIO_Port, PROFET_IN1_Pin, enable ? GPIO_PIN_SET : GPIO_PIN_RESET);
	}
}

#if TEST == 1

void SW_tim_lght_cb( void )
{
	__disable_irq();
	uint8_t idx = 0; // this line

	if(SW1state)
	{
		SW_setState(idx, SW1state);
		SW1state = false;
	}
	else
	{
		SW_setState(idx, SW1state);
		SW1state = true;
	}
	__enable_irq();
}

void SW_tim_cb( void )
{
	__disable_irq();
	counter++;
	SW_setState(1, counter < compare); //this line
	//SW_setState(1, counter < compare);
	counter = counter % 100;
	__enable_irq();
}

#else
void SW_tim_lght_cb( void )
{
	__disable_irq();
	uint8_t idx = 1; // this line

	if(SW1state)
	{
		SW_setState(idx, SW1state);
		SW1state = false;
	}
	else
	{
		SW_setState(idx, SW1state);
		SW1state = true;
	}
	__enable_irq();
}

void SW_tim_cb( void )
{
	__disable_irq();
	counter++;
	SW_setState(0, counter < compare); //this line
	//SW_setState(1, counter < compare);
	counter = counter % 100;
	__enable_irq();
}
#endif
