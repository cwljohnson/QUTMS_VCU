/*
 * utilities.c
 *
 *  Created on: Feb 9, 2022
 *      Author: Calvin
 */

#include "utilities.h"

double map_capped(uint16_t input, uint16_t in_min, uint16_t in_max, uint16_t out_min, uint16_t out_max) {
	if (input < in_min) {
		input = in_min;
	}
	else if (input > in_max) {
		input = in_max;
	}

	return (double) (input - in_min) * (double) (out_max - out_min) / (double) (in_max - in_min) + (double) out_min;
}
