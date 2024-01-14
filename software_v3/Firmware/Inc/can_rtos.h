/*
 * can_rtos.h
 *
 *  Created on: Nov 27, 2023
 *      Author: Calvin
 */

#ifndef INC_CAN_RTOS_H_
#define INC_CAN_RTOS_H_

#include <stdint.h>
#include <stdbool.h>
#include "can.h"
#include "cmsis_os.h"

#define CAN_QUEUE_RX_SIZE (20U)
#define CAN_QUEUE_TX_SIZE (10U)

enum CAN_SRC {
	CAN_SRC_CAN1 = 0,
	CAN_SRC_CAN2
};

#if QEV3 == 1
#define CAN_SRC_SENSOR CAN_SRC_CAN1
#define CAN_SRC_CTRL CAN_SRC_CAN1
#endif

#if QEV4 == 1
#define CAN_SRC_SENSOR CAN_SRC_CAN2
#define CAN_SRC_CTRL CAN_SRC_CAN2
#endif

typedef struct can_msg {
	uint8_t src;
	uint32_t id;
	uint8_t ide;
	uint8_t dlc;
	uint8_t data[8];
} can_msg_t;

extern osMessageQueueId_t q_can_rx;

void can_rtos_setup(void);
bool init_CAN1();
bool init_CAN2();

void can_process_tx(void);

void can_rx_enqueue(CAN_RxHeaderTypeDef *header, uint8_t data[8], uint8_t src);
void can_tx_enqueue(can_msg_t *msg, uint8_t dest);

void can1_rx_interrupt_cb(uint32_t fifo);
void can2_rx_interrupt_cb(uint32_t fifo);

void can1_tx_interrupt_cb();
void can2_tx_interrupt_cb();

#endif /* INC_CAN_RTOS_H_ */
