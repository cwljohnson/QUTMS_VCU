/*
 * sensor_dashLED.h
 *
 *  Created on: Dec 4, 2023
 *      Author: Calvin
 */

#ifndef INC_SENSOR_DASHLED_H_
#define INC_SENSOR_DASHLED_H_

#include <stdbool.h>
#include <stdint.h>

#define DASH_CH_LED_BSPD 0
#define DASH_CH_LED_IMD 7
#define DASH_CH_LED_AMS 6
#define DASH_CH_LED_PDOC 1

void sensor_setup_dashLED(void);
void sensor_dashLED_setState(uint8_t idx, bool on);

void sensor_dashLED_setLatchBypass(bool bypass);

#endif /* INC_SENSOR_DASHLED_H_ */
