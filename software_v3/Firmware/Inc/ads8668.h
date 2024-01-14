/*
 * ads8668.h
 *
 *  Created on: Feb 6, 2023
 *      Author: Calvin
 */

#ifndef INC_ADS8668_H_
#define INC_ADS8668_H_

#include <stdint.h>

#define ADS8668_NUM_CHANNELS	8

#define ADS8668_RANGE_10V24_DIFF	0b0000
#define ADS8668_RANGE_5V12_DIFF		0b0001
#define ADS8668_RANGE_2V56_DIFF		0b0010
#define ADS8668_RANGE_1V28_DIFF		0b0011
#define ADS8668_RANGE_0V64_DIFF		0b1011
#define ADS8668_RANGE_10V24			0b0101
#define ADS8668_RANGE_5V12			0b0110
#define ADS8668_RANGE_2V56			0b0111
#define ADS8668_RANGE_1V28			0b1111

#define ADS8668_CMD_NO_OP		0x00
#define ADS8668_CMD_STDBY		0x82
#define ADS8668_CMD_PWR_DN		0x83
#define ADS8668_CMD_RST			0x85
#define ADS8668_CMD_AUTO_RST	0xA0

#define ADS8668_CMD_MAN_CH_0 	0xC0
#define ADS8668_CMD_MAN_CH_1 	0xC4
#define ADS8668_CMD_MAN_CH_2 	0xC8
#define ADS8668_CMD_MAN_CH_3 	0xCC
#define ADS8668_CMD_MAN_CH_4 	0xD0
#define ADS8668_CMD_MAN_CH_5 	0xD4
#define ADS8668_CMD_MAN_CH_6 	0xD8
#define ADS8668_CMD_MAN_CH_7 	0xDC

#define ADS8668_REG_FEAT_SEL	0x03

typedef struct ADS8668_cmd_ret {
	uint16_t value;
	uint8_t channel_address;
} ADS8668_cmd_ret_t;

void ADS8668_CS_ON();
void ADS8668_CS_OFF();

void ADS8668_Reset();
void ADS8668_Init();

ADS8668_cmd_ret_t ADS8668_WriteCmd(uint8_t cmdReg);

void ADS8668_WriteCmd_DMA(uint8_t cmdReg);
ADS8668_cmd_ret_t ADS8668_DMA_result_WriteCMD();

uint8_t ADS8668_WriteReg(uint8_t reg, uint8_t data);
uint8_t ADS8668_ReadReg(uint8_t reg);
void ADS8668_SetRange(uint8_t idx, uint8_t range);


#endif /* INC_ADS8668_H_ */
