/*
 * sensor_shutdown.h
 *
 *  Created on: Oct 13, 2023
 *      Author: Calvin
 */

#ifndef INC_SENSOR_SHUTDOWN_H_
#define INC_SENSOR_SHUTDOWN_H_

#include "sensor.h"
#include "main.h"

#include <stdbool.h>
#include <stdint.h>

#define SENSOR_IDX_P_SHDN_0 (1)
#define SENSOR_IDX_P_SHDN_1 (0)
#define SENSOR_IDX_P_SHDN_2 (6)
#define SENSOR_IDX_P_SHDN_3 (7)

#if QEV4 == 1
#define SENSOR_IDX_P_SHDN_STAT (3)
#else
#define SENSOR_IDX_P_SHDN_STAT (5)
#endif

#define CAN_PHASE_S_SHUTDOWN (2)

typedef struct {
	uint8_t segs[4];
	bool state;
} shutdown_status_t;

extern shutdown_status_t shutdown_status;

void sensor_setup_shutdown(void);
void sensor_update_shutdown(sensor_data_t *sensor_data);
void sensor_tx_shutdown(void);

#endif /* INC_SENSOR_SHUTDOWN_H_ */
