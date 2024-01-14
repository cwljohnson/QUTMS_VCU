/*
 * p_isrc.c
 *
 *  Created on: 3 Feb. 2022
 *      Author: Calvin J
 */

#include "p_isrc.h"

bool setup_ISRC() {
	MAX5548_init(MAX5548_IDX_1);
	MAX5548_init(MAX5548_IDX_2);
	MAX5548_init(MAX5548_IDX_3);
	MAX5548_init(MAX5548_IDX_4);

	for(int i = 0; i < 8; i++) {
		isrc_SetCurrentEnabled(i, false);
	}

	return true;
}

void isrc_GetMapping(uint8_t idx, uint8_t *maxIdx, uint8_t *channel) {
	if (idx == 0) {
		*maxIdx = MAX5548_IDX_4;
		*channel = MAX5548_CH_B;
	}
	else if (idx == 1) {
		*maxIdx = MAX5548_IDX_4;
		*channel = MAX5548_CH_A;
	}
	else if (idx == 2) {
		*maxIdx = MAX5548_IDX_1;
		*channel = MAX5548_CH_A;
	}
	else if (idx == 3) {
		*maxIdx = MAX5548_IDX_1;
		*channel = MAX5548_CH_B;
	}
	else if (idx == 4) {
		*maxIdx = MAX5548_IDX_2;
		*channel = MAX5548_CH_A;
	}
	else if (idx == 5) {
		*maxIdx = MAX5548_IDX_2;
		*channel = MAX5548_CH_B;
	}
	else if (idx == 6) {
		*maxIdx = MAX5548_IDX_3;
		*channel = MAX5548_CH_B;
	}
	else if (idx == 7) {
		*maxIdx = MAX5548_IDX_3;
		*channel = MAX5548_CH_A;
	}
}

void isrc_SetCurrentScale(uint8_t idx, uint8_t scale) {
	uint8_t maxIdx;
	uint8_t channel;
	isrc_GetMapping(idx, &maxIdx, &channel);

	MAX5548_SetChannelScale(maxIdx, channel, scale);
}

void isrc_SetCurrentValue(uint8_t idx, uint8_t value) {
	uint8_t maxIdx;
	uint8_t channel;
	isrc_GetMapping(idx, &maxIdx, &channel);

	MAX5548_SetChannelValue(maxIdx, channel, value);
}

void isrc_SetCurrentEnabled(uint8_t idx, bool enabled) {
	uint8_t maxIdx;
	uint8_t channel;
	isrc_GetMapping(idx, &maxIdx, &channel);

	MAX5548_EnableChannel(maxIdx, channel, enabled);
}
