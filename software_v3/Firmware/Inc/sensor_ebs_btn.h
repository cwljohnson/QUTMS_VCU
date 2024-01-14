/*
 * sensor_ebs_btn.h
 *
 *  Created on: Dec 15, 2023
 *      Author: Calvin
 */

#ifndef INC_SENSOR_EBS_BTN_H_
#define INC_SENSOR_EBS_BTN_H_

#include "sensor.h"
#include "main.h"
#include "state_machine.h"

#include <stdint.h>
#include <stdbool.h>

#define SENSOR_IDX_P_EBS_BTN 3
#define SW_IDX_P_EBS_BTN (1)

void sensor_setup_ebs_btn(void);
void sensor_update_ebs_btn(sensor_data_t *sensor_data);
void sensor_ebs_btn_update_outputs(state_machine_t *state_machine);


#endif /* INC_SENSOR_EBS_BTN_H_ */
