/*
 * s_dashLED.h
 *
 *  Created on: 21 Feb. 2022
 *      Author: Calvin J
 */

#ifndef INC_S_DASHLED_H_
#define INC_S_DASHLED_H_

#include <stdbool.h>
#include <stdint.h>

#define DASH_CH_LED_BSPD 0
#define DASH_CH_LED_IMD 7
#define DASH_CH_LED_AMS 6
#define DASH_CH_LED_PDOC 1

void setup_dashLED();
void dashLED_setState(uint8_t idx, bool on);

#endif /* INC_S_DASHLED_H_ */
