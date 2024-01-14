/*
 * p_12vSW.h
 *
 *  Created on: Sep 7, 2023
 *      Author: Calvin
 */

#ifndef INC_P_12VSW_H_
#define INC_P_12VSW_H_

#include <stdbool.h>
#include <stdint.h>

void SW_setState(uint8_t idx, bool enable);
void SW_setPWM(uint8_t idx, uint8_t duty_cycle);

void pwm12V_timer_cb(void);

#endif /* INC_P_12VSW_H_ */
