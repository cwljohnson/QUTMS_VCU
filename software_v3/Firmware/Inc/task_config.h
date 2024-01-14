/*
 * task_config.h
 *
 *  Created on: Aug 5, 2023
 *      Author: Calvin
 */

#ifndef INC_TASK_CONFIG_H_
#define INC_TASK_CONFIG_H_

#include <stdint.h>
#include <stdbool.h>

typedef enum {
	C_IDX_P_ACCEL_MIN,
	C_IDX_P_ACCEL_MAX
} CONFIG_IDX;

typedef enum {
	CT_UINT16,
	CT_INT16,
	CT_UINT32,
	CT_INT32
} CONFIG_TYPE;

typedef union {
	uint16_t u16;
	int16_t i16;
	uint32_t u32;
	int32_t i32;
} config_value_u;

typedef struct config_item {
	config_value_u minValue;
	config_value_u maxValue;
	void *address;
	bool enabled;
	CONFIG_TYPE type;
} config_item_t;

void setup_task_config(void);

#endif /* INC_TASK_CONFIG_H_ */
