/*
 * inv_vesc.h
 *
 *  Created on: Dec 11, 2023
 *      Author: Calvin
 */

#ifndef INC_INV_VESC_H_
#define INC_INV_VESC_H_

#include <stdint.h>
#include <stdbool.h>
#include <CAN_VESC.h>

#include "can_rtos.h"
#include "main.h"

#if QEV3 == 1
#define CAN_PHASE_INV (2U)

typedef struct {
	uint8_t id;
	float current_request;
	int32_t motor_rpm;
	float motor_kmh;
} vesc_t;

typedef struct {
	vesc_t vesc[NUM_VESC];
	float max_current;
	float max_regen_current;
	float regen_kmh_cutoff;
} inv_vesc_t;

extern inv_vesc_t inverters;

void inv_vesc_setup(void);

void inv_vesc_tx(void);
void inv_vesc_tx_shdn(uint8_t id);
void inv_vesc_tx_req(vesc_t* inverter);

#endif

#endif /* INC_INV_VESC_H_ */
