/*
 * sensor_rtd.h
 *
 *  Created on: Sep 7, 2023
 *      Author: Calvin
 */

#ifndef INC_SENSOR_RTD_H_
#define INC_SENSOR_RTD_H_

#include <stdint.h>
#include <stdbool.h>
#include "state_machine.h"
#include "sensor.h"

#define SENSOR_IDX_P_RTD_BTN 6
#define SW_IDX_P_RTD_BTN 0
#define SW_IDX_P_RTD_SIREN 1

void sensor_rtd_update_outputs(state_machine_t *state_machine);



void sensor_setup_rtd_btn(void);
void sensor_update_rtd_btn(sensor_data_t *sensor_data);

bool rtd_get_btn(void);

#endif /* INC_SENSOR_RTD_H_ */
