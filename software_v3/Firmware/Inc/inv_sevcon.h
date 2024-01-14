/*
 * inv_sevcon.h
 *
 *  Created on: Nov 16, 2023
 *      Author: Calvin
 */

#ifndef INC_INV_SEVCON_H_
#define INC_INV_SEVCON_H_

#include <stdint.h>
#include <stdbool.h>

#include <CAN_SEVCON.h>

#include "can_rtos.h"
#include "main.h"

#if QEV4 == 1

#define CAN_PHASE_INV (2U)
#define NUM_INV (2U)

typedef struct
{
	int16_t motor_speed;
	int16_t battery_current;
	int16_t temp_heat;
	int16_t temp_remaining;
	int16_t cap_voltage;
	sevcon_state_t status_word;
	uint16_t fault_code;
} sevcon_data_t;

typedef struct
{
	float torque_demand;
	sevcon_cmd_t control_word;
	float torque_limit_drive;
	float torque_limit_regen;
	int16_t speed_limit_forward;
	int16_t speed_limit_soft;
	int16_t speed_limit_backward;
	int16_t current_limit_discharge; // > 0A
	int16_t current_limit_charge;	 // < 0A
	float target_cap_voltage;
} sevcon_ctrl_t;

typedef struct
{
	uint8_t address;
	sevcon_data_t data;
	sevcon_ctrl_t ctrl;
	bool alive;
	uint32_t last_time;
} sevcon_t;

typedef struct
{
	sevcon_t sevcon[NUM_INV];
	float max_torque;
} inv_sevcon_t;

extern inv_sevcon_t inverters;

void inv_sevcon_setup(void);

bool inv_sevcon_check_msg(can_msg_t *msg);

void inv_sevcon_tx(void);
void inv_sevcon_tx_hc(sevcon_t *inverter);
void inv_sevcon_tx_msg(sevcon_hs_t *hs_msg);
#endif

#endif /* INC_INV_SEVCON_H_ */
