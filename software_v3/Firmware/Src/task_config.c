/*
 * task_config.c
 *
 *  Created on: Aug 5, 2023
 *      Author: Calvin
 */

#include "task_config.h"

const config_item_t config_items[16] = {
		[C_IDX_P_ACCEL_MIN] = {
				.address = 0,
				.type = CT_UINT16,
				.minValue = { .u16 = 0 },
				.maxValue = { .u16 = 0xFF }
		},
		[C_IDX_P_ACCEL_MAX] = {
				.address = 0,
				.type = CT_UINT16,
				.minValue = { .u16 = 0 },
				.maxValue = { .u16 = 0xFF }
		}
};

void setup_task_config(void) {
	for (uint16_t i = 0; i < 16; i++) {
		volatile void *p = config_items[i].address;
	}
}
