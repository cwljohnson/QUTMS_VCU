/*
 * can_rtos.c
 *
 *  Created on: Nov 27, 2023
 *      Author: Calvin
 */

#include "can_rtos.h"
#include "task_main.h"
#include "comms.h"

osMessageQueueId_t q_can_rx;
static StaticQueue_t cb_q_can_rx;
static uint32_t mq_q_can_rx[(CAN_QUEUE_RX_SIZE * sizeof(can_msg_t)) / 4U]; // must be 4 byte aligned
const osMessageQueueAttr_t attr_q_can_rx = { .name = "CAN Rx", .cb_size =
		sizeof(cb_q_can_rx), .cb_mem = &cb_q_can_rx, .mq_size =
		sizeof(mq_q_can_rx), .mq_mem = mq_q_can_rx };

osMessageQueueId_t q_can_tx1;
static StaticQueue_t cb_q_can_tx1;
static uint32_t mq_q_can_tx1[(CAN_QUEUE_TX_SIZE * sizeof(can_msg_t)) / 4U]; // must be 4 byte aligned
const osMessageQueueAttr_t attr_q_can_tx1 = { .name = "CAN Tx1", .cb_size =
		sizeof(cb_q_can_tx1), .cb_mem = &cb_q_can_tx1, .mq_size =
		sizeof(mq_q_can_tx1), .mq_mem = mq_q_can_tx1 };

osMessageQueueId_t q_can_tx2;
static StaticQueue_t cb_q_can_tx2;
static uint32_t mq_q_can_tx2[(CAN_QUEUE_TX_SIZE * sizeof(can_msg_t)) / 4U]; // must be 4 byte aligned
const osMessageQueueAttr_t attr_q_can_tx2 = { .name = "CAN Tx2", .cb_size =
		sizeof(cb_q_can_tx2), .cb_mem = &cb_q_can_tx2, .mq_size =
		sizeof(mq_q_can_tx2), .mq_mem = mq_q_can_tx2 };

void can_tx_transmit(can_msg_t *msg, CAN_HandleTypeDef *hcan);

void can_rtos_setup(void) {
	q_can_rx = osMessageQueueNew(CAN_QUEUE_RX_SIZE, sizeof(can_msg_t),
			&attr_q_can_rx);

	q_can_tx1 = osMessageQueueNew(CAN_QUEUE_TX_SIZE, sizeof(can_msg_t),
			&attr_q_can_tx1);
	q_can_tx2 = osMessageQueueNew(CAN_QUEUE_TX_SIZE, sizeof(can_msg_t),
			&attr_q_can_tx2);
}

void can_process_tx(void) {
	static can_msg_t tx_msg;

	// if messages are queued up to TX and theres free slots, add to the TX to transmit
	if (osMessageQueueGetCount(q_can_tx1) > 0) {
		// message is pending
		// disable CAN1 tx interrupt
		HAL_NVIC_DisableIRQ(CAN1_TX_IRQn);

		if (HAL_CAN_GetTxMailboxesFreeLevel(&hcan1) > 0) {
			if (osMessageQueueGet(q_can_tx1, &tx_msg, NULL, 0) == osOK) {
				can_tx_transmit(&tx_msg, &hcan1);
			}
		}

		// enable CAN1 tx interrupt
		HAL_NVIC_EnableIRQ(CAN1_TX_IRQn);
	}

	if (osMessageQueueGetCount(q_can_tx2) > 0) {
		// message is pending
		// disable CAN2 tx interrupt
		HAL_NVIC_DisableIRQ(CAN2_TX_IRQn);

		if (HAL_CAN_GetTxMailboxesFreeLevel(&hcan2) > 0) {
			if (osMessageQueueGet(q_can_tx2, &tx_msg, NULL, 0) == osOK) {
				can_tx_transmit(&tx_msg, &hcan2);
			}
		}

		// enable CAN2 tx interrupt
		HAL_NVIC_EnableIRQ(CAN2_TX_IRQn);
	}
}

void can_rx_enqueue(CAN_RxHeaderTypeDef *header, uint8_t data[8], uint8_t src) {
	can_msg_t msg;
	msg.src = src;
	msg.dlc = header->DLC;
	msg.ide = header->IDE;

	if (msg.ide == CAN_ID_EXT) {
		msg.id = header->ExtId;
	}
	else {
		msg.id = header->StdId;
	}

	for (uint8_t i = 0; i < msg.dlc; i++) {
		msg.data[i] = data[i];
	}

	// TODO: special check here for if shutdown message
	// need to alert inverter stuff if possible

	// add to rx queue
	osMessageQueuePut(q_can_rx, &msg, 0U, 0U);

	// only explicitly flag when theres a few messages
	if (osMessageQueueGetCount(q_can_rx) > 10) {
		// set event flag to let main task know message is ready for processing
		osThreadFlagsSet(th_tsk_main, MAIN_EVT_FLAG_CAN_RX);
	}

}

void can_tx_enqueue(can_msg_t *msg, uint8_t dest) {
	msg->src = dest;

	if (dest == CAN_SRC_CAN1) {
		HAL_NVIC_DisableIRQ(CAN1_TX_IRQn);

		// can we send immediately?
		if (HAL_CAN_GetTxMailboxesFreeLevel(&hcan1) > 0) {
			can_tx_transmit(msg, &hcan1);
		}
		else {
			osMessageQueuePut(q_can_tx1, msg, 0U, 0U);

			// set event flag to let main task know message is ready for transmission
			osThreadFlagsSet(th_tsk_main, MAIN_EVT_FLAG_CAN_TX);
		}

		// enable CAN1 tx interrupt
		HAL_NVIC_EnableIRQ(CAN1_TX_IRQn);

	}
	else {
		HAL_NVIC_DisableIRQ(CAN2_TX_IRQn);

		// can we send immediately?
		if (HAL_CAN_GetTxMailboxesFreeLevel(&hcan2) > 0) {
			can_tx_transmit(msg, &hcan2);
		}
		else {
			osMessageQueuePut(q_can_tx2, msg, 0U, 0U);

			// set event flag to let main task know message is ready for transmission
			osThreadFlagsSet(th_tsk_main, MAIN_EVT_FLAG_CAN_TX);
		}

		// enable CAN1 tx interrupt
		HAL_NVIC_EnableIRQ(CAN2_TX_IRQn);
	}
}

void can_tx_transmit(can_msg_t *msg, CAN_HandleTypeDef *hcan) {
	CAN_TxHeaderTypeDef header;
	uint32_t mailbox;

	header.IDE = msg->ide;
	header.DLC = msg->dlc;
	header.ExtId = msg->id;
	header.StdId = msg->id;
	header.RTR = CAN_RTR_DATA;

	HAL_StatusTypeDef ret = HAL_CAN_AddTxMessage(hcan, &header, msg->data,
			&mailbox);
}

bool init_CAN1() {

	if (HAL_CAN_Start(&hcan1) != HAL_OK) {
		// TODO: print
		// printf("ERROR: FAILED TO START CAN1");
		return false;
	}

	CAN_FilterTypeDef sFilterConfig;

	sFilterConfig.FilterBank = 0;
	sFilterConfig.FilterMode = CAN_FILTERMODE_IDMASK;
	sFilterConfig.FilterScale = CAN_FILTERSCALE_32BIT;
	sFilterConfig.FilterIdHigh = 0x0000;
	sFilterConfig.FilterIdLow = 0x0001;
	sFilterConfig.FilterMaskIdHigh = 0x0000;
	sFilterConfig.FilterMaskIdLow = 0x0001;
	sFilterConfig.FilterFIFOAssignment = CAN_RX_FIFO0;
	sFilterConfig.FilterActivation = ENABLE;

	if (HAL_CAN_ConfigFilter(&hcan1, &sFilterConfig) != HAL_OK) {
		// TODO: print
		// printf("ERROR: FAILED TO CONFIG FILTER ON CAN1");
		return false;
	}

	if (HAL_CAN_ActivateNotification(&hcan1, CAN_IT_RX_FIFO0_MSG_PENDING)
			!= HAL_OK) {
		// TODO: print
		// printf("ERROR: Failed activate CAN1 notification on RX0");
		return false;
	}

	if (HAL_CAN_ActivateNotification(&hcan1, CAN_IT_RX_FIFO1_MSG_PENDING)
			!= HAL_OK) {
		// TODO: print
		// printf("ERROR: Failed activate CAN1 notification on RX0");
		return false;
	}

	if (HAL_CAN_ActivateNotification(&hcan1, CAN_IT_TX_MAILBOX_EMPTY)
			!= HAL_OK) {
		// TODO: print
		// printf("ERROR: Failed activate CAN1 notification on RX0");
		return false;
	}

	return true;
}

bool init_CAN2() {

	if (HAL_CAN_Start(&hcan2) != HAL_OK) {
		// TODO: print
		// printf("ERROR: FAILED TO START CAN1");
		return false;
	}

	CAN_FilterTypeDef sFilterConfig;

	sFilterConfig.FilterBank = 0;
	sFilterConfig.FilterMode = CAN_FILTERMODE_IDMASK;
	sFilterConfig.FilterScale = CAN_FILTERSCALE_32BIT;
	sFilterConfig.FilterIdHigh = 0x0000;
	sFilterConfig.FilterIdLow = 0x0001;
	sFilterConfig.FilterMaskIdHigh = 0x0000;
	sFilterConfig.FilterMaskIdLow = 0x0001;
	sFilterConfig.FilterFIFOAssignment = CAN_RX_FIFO0;
	sFilterConfig.FilterActivation = ENABLE;

	if (HAL_CAN_ConfigFilter(&hcan2, &sFilterConfig) != HAL_OK) {
		// TODO: print
		// printf("ERROR: FAILED TO CONFIG FILTER ON CAN1");
		return false;
	}

	if (HAL_CAN_ActivateNotification(&hcan2, CAN_IT_RX_FIFO0_MSG_PENDING)
			!= HAL_OK) {
		// TODO: print
		// printf("ERROR: Failed activate CAN1 notification on RX0");
		return false;
	}

	if (HAL_CAN_ActivateNotification(&hcan2, CAN_IT_RX_FIFO1_MSG_PENDING)
			!= HAL_OK) {
		// TODO: print
		// printf("ERROR: Failed activate CAN1 notification on RX0");
		return false;
	}

	if (HAL_CAN_ActivateNotification(&hcan2, CAN_IT_TX_MAILBOX_EMPTY)
			!= HAL_OK) {
		// TODO: print
		// printf("ERROR: Failed activate CAN1 notification on RX0");
		return false;
	}

	return true;
}

void can1_rx_interrupt_cb(uint32_t fifo) {
	static CAN_RxHeaderTypeDef header;
	static uint8_t data[8];

	// TODO: test this interrupt stays fired if multiple messages

	HAL_StatusTypeDef ret = HAL_CAN_GetRxMessage(&hcan1, fifo, &header, data);

	if (ret == HAL_OK) {
		// grab latest message
		can_rx_enqueue(&header, data, CAN_SRC_CAN1);
	}
}

void can2_rx_interrupt_cb(uint32_t fifo) {
	static CAN_RxHeaderTypeDef header;
	static uint8_t data[8];

	// TODO: test this interrupt stays fired if multiple messages

	HAL_StatusTypeDef ret = HAL_CAN_GetRxMessage(&hcan2, fifo, &header, data);

	if (ret == HAL_OK) {
		// grab latest message
		can_rx_enqueue(&header, data, CAN_SRC_CAN2);
	}
}

void can1_tx_interrupt_cb() {
	static can_msg_t msg;

	// just finished transmission, check if theres any more messages and transmit if needed
	// just finished therefore must have free slot thems the rules
	if (HAL_CAN_GetTxMailboxesFreeLevel(&hcan1) > 0) {
		if (osMessageQueueGetCount(q_can_tx1) > 0) {
			if (osMessageQueueGet(q_can_tx1, &msg, NULL, 0) == osOK) {
				can_tx_transmit(&msg, &hcan1);
			}
		}
	}
}

void can2_tx_interrupt_cb() {
	static can_msg_t msg;

	// just finished transmission, check if theres any more messages and transmit if needed
	// just finished therefore must have free slot thems the rules
	if (HAL_CAN_GetTxMailboxesFreeLevel(&hcan2) > 0) {
		if (osMessageQueueGetCount(q_can_tx2) > 0) {
			if (osMessageQueueGet(q_can_tx2, &msg, NULL, 0) == osOK) {
				can_tx_transmit(&msg, &hcan2);
			}
		}
	}
}
