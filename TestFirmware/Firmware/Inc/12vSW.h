/*
 * 12vSW.h
 *
 *  Created on: Oct 10, 2021
 *      Author: Calvin
 */

#ifndef INC_12VSW_H_
#define INC_12VSW_H_

#include <stdbool.h>
#include <stdint.h>


void SW_setState(uint8_t idx, bool enable);
void SW_tim_cb( void );
void SW_tim_lght_cb( void );

#endif /* INC_12VSW_H_ */
