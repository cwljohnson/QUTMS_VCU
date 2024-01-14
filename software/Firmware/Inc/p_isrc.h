/*
 * p_isrc.h
 *
 *  Created on: 3 Feb. 2022
 *      Author: Calvin J
 */

#ifndef INC_P_ISRC_H_
#define INC_P_ISRC_H_

#include <stdbool.h>
#include <stdint.h>

#include "max5548.h"

bool setup_ISRC();

void isrc_SetCurrentScale(uint8_t idx, uint8_t scale);
void isrc_SetCurrentValue(uint8_t idx, uint8_t value);
void isrc_SetCurrentEnabled(uint8_t idx, bool enabled);



#endif /* INC_P_ISRC_H_ */
