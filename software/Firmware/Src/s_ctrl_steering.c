/*
 * s_ctrl_steering.c
 *
 *  Created on: Feb 24, 2022
 *      Author: Calvin J
 */

#include "s_ctrl_steering.h"
#include "can.h"
#include <stdint.h>
#include <CAN_VCU.h>

steering_values_t ctrl_steering_values;
ms_timer_t timer_ctrl_steering;
bool steering_timeout;

#if VCU_CURRENT_ID == VCU_ID_CTRL

void setup_ctrl_steering() {
	ctrl_steering_values.steering_imp_start = HAL_GetTick();

	timer_ctrl_steering = timer_init(25, true, ctrl_steering_timer_cb);
	timer_start(&timer_ctrl_steering);
}

void ctrl_steering_timer_cb(void *args) {
	CAN_MSG_Generic_t msg;
	while (queue_next(&queue_CAN, &msg)) {
		if (msg.ID == VCU_TransmitSteering_ID) {
			int16_t steer0;
			int16_t steer1;
			uint16_t adcSteer0;
			uint16_t adcSteer1;
			Parse_VCU_TransmitSteering(msg.data, &steer0, &steer1, &adcSteer0, &adcSteer1);

			ctrl_steering_values.steering_mapped[0] = steer0 / 10.0;
			ctrl_steering_values.steering_mapped[1] = steer1 / 10.0;

			ctrl_steering_values.steering_imp_start = HAL_GetTick();
			steering_timeout = false;
		}
	}

	if ((HAL_GetTick() - ctrl_steering_values.steering_imp_start) > STEERING_IMPLAUSIBILTIY_TIMEOUT) {
		steering_timeout = true;
	}
}

#endif
