/*
 * util.c
 *
 *  Created on: Nov 26, 2023
 *      Author: Calvin
 */

#include "util.h"

float adcToAngle(uint16_t adc) {
	if (adc < 100) {
		return 0;
	}

	float angle = ((adc - 2500) * (60.0)) / (4000.0);
	return angle;
}
