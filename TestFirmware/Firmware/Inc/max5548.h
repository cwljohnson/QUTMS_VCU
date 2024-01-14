/*
 * max5548.h
 *
 *  Created on: 23 Feb. 2022
 *      Author: Calvin
 */

#ifndef INC_MAX5548_H_
#define INC_MAX5548_H_

#include <stdint.h>
#include <stdbool.h>

#define MAX5548_IDX_1 0
#define MAX5548_IDX_2 1
#define MAX5548_IDX_3 2
#define MAX5548_IDX_4 3

#define MAX5548_CH_A 0
#define MAX5548_CH_B 1


#define MAX5548_CMD_SCALE_A			0x10
#define MAX5548_CMD_SCALE_B			0x30
#define MAX5548_SCALE_0 			0x10 // 1mA-2mA
#define MAX5548_SCALE_1 			0x01 // 1.5mA-3mA
#define MAX5548_SCALE_2 			0x02 // 2.5mA-5mA
#define MAX5548_SCALE_3 			0x03 // 4.5mA-9mA
#define MAX5548_SCALE_4 			0x04 // 8mA-16mA
#define MAX5548_SCALE_5 			0x05 // 15mA-30mA

#define MAX5548_CMD_NO_OP			0x00
#define MAX5548_CMD_LOAD_DAC_BOTH	0x01
#define MAX5548_CMD_LOAD_DAC_A		0x02
#define MAX5548_CMD_LOAD_DAC_B		0x03
#define MAX5548_CMD_LOAD_SHIFT_BOTH	0x04
#define MAX5548_CMD_LOAD_SHIFT_A	0x05
#define MAX5548_CMD_LOAD_SHIFT_B	0x06
#define MAX5548_CMD_UPDATE_BOTH		0x07
#define MAX5548_CMD_UPDATE_A		0x09
#define MAX5548_CMD_UPDATE_B		0x0A
#define MAX5548_CMD_REF_INTERNAL 	0x0B
#define MAX5548_CMD_REF_EXTERNAL 	0x0C
#define MAX5548_CMD_SHUTDOWN_BOTH	0x0D
#define MAX5548_CMD_SHUTDOWN_A		0x0E
#define MAX5548_CMD_SHUTDOWN_B		0x0F
#define MAX5548_CMD_POWER_BOTH		0x2D
#define MAX5548_CMD_POWER_A			0x2E
#define MAX5548_CMD_POWER_B			0x2F

void MAX5548_CS_ON(uint8_t idx);
void MAX5548_CS_OFF(uint8_t idx);
void MAX5548_EnableChannel(uint8_t idx, uint8_t channel, bool value);
void MAX5548_SetChannelScale(uint8_t idx, uint8_t channel, uint8_t scale);
void MAX5548_SetChannelValue(uint8_t idx, uint8_t channel, uint8_t value);
void MAX5548_SendCMD(uint8_t idx, uint8_t cmd);
void MAX5548_SendDATA(uint8_t idx, uint8_t cmd, uint8_t data);

void MAX5548_init(uint8_t idx);



#endif /* INC_MAX5548_H_ */
