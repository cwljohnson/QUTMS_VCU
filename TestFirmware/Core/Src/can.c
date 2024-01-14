/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    can.c
  * @brief   This file provides code for the configuration
  *          of the CAN instances.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "can.h"

/* USER CODE BEGIN 0 */
uint32_t txMailbox_CAN1 = 0;
uint32_t txMailbox_CAN2 = 0;
message_queue_t queue_CAN1_Tx;
message_queue_t queue_CAN2_Tx;
message_queue_t queue_CAN;
uint32_t can1Mailbox;
uint32_t can1Mailbox;

message_queue_t queue_CAN1_Rx;
message_queue_t queue_CAN2_Rx;
/* USER CODE END 0 */

CAN_HandleTypeDef hcan1;
CAN_HandleTypeDef hcan2;

/* CAN1 init function */
void MX_CAN1_Init(void)
{

  /* USER CODE BEGIN CAN1_Init 0 */

  /* USER CODE END CAN1_Init 0 */

  /* USER CODE BEGIN CAN1_Init 1 */

  /* USER CODE END CAN1_Init 1 */
  hcan1.Instance = CAN1;
  hcan1.Init.Prescaler = 4;
  hcan1.Init.Mode = CAN_MODE_NORMAL;
  hcan1.Init.SyncJumpWidth = CAN_SJW_1TQ;
  hcan1.Init.TimeSeg1 = CAN_BS1_12TQ;
  hcan1.Init.TimeSeg2 = CAN_BS2_2TQ;
  hcan1.Init.TimeTriggeredMode = DISABLE;
  hcan1.Init.AutoBusOff = DISABLE;
  hcan1.Init.AutoWakeUp = DISABLE;
  hcan1.Init.AutoRetransmission = DISABLE;
  hcan1.Init.ReceiveFifoLocked = DISABLE;
  hcan1.Init.TransmitFifoPriority = DISABLE;
  if (HAL_CAN_Init(&hcan1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN CAN1_Init 2 */

  /* USER CODE END CAN1_Init 2 */

}
/* CAN2 init function */
void MX_CAN2_Init(void)
{

  /* USER CODE BEGIN CAN2_Init 0 */

  /* USER CODE END CAN2_Init 0 */

  /* USER CODE BEGIN CAN2_Init 1 */

  /* USER CODE END CAN2_Init 1 */
  hcan2.Instance = CAN2;
  hcan2.Init.Prescaler = 4;
  hcan2.Init.Mode = CAN_MODE_NORMAL;
  hcan2.Init.SyncJumpWidth = CAN_SJW_1TQ;
  hcan2.Init.TimeSeg1 = CAN_BS1_12TQ;
  hcan2.Init.TimeSeg2 = CAN_BS2_2TQ;
  hcan2.Init.TimeTriggeredMode = DISABLE;
  hcan2.Init.AutoBusOff = DISABLE;
  hcan2.Init.AutoWakeUp = DISABLE;
  hcan2.Init.AutoRetransmission = DISABLE;
  hcan2.Init.ReceiveFifoLocked = DISABLE;
  hcan2.Init.TransmitFifoPriority = DISABLE;
  if (HAL_CAN_Init(&hcan2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN CAN2_Init 2 */

  /* USER CODE END CAN2_Init 2 */

}

static uint32_t HAL_RCC_CAN1_CLK_ENABLED=0;

void HAL_CAN_MspInit(CAN_HandleTypeDef* canHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(canHandle->Instance==CAN1)
  {
  /* USER CODE BEGIN CAN1_MspInit 0 */

  /* USER CODE END CAN1_MspInit 0 */
    /* CAN1 clock enable */
    HAL_RCC_CAN1_CLK_ENABLED++;
    if(HAL_RCC_CAN1_CLK_ENABLED==1){
      __HAL_RCC_CAN1_CLK_ENABLE();
    }

    __HAL_RCC_GPIOB_CLK_ENABLE();
    /**CAN1 GPIO Configuration
    PB8     ------> CAN1_RX
    PB9     ------> CAN1_TX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_8|GPIO_PIN_9;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF9_CAN1;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* CAN1 interrupt Init */
    HAL_NVIC_SetPriority(CAN1_TX_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(CAN1_TX_IRQn);
    HAL_NVIC_SetPriority(CAN1_RX1_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(CAN1_RX1_IRQn);
  /* USER CODE BEGIN CAN1_MspInit 1 */

  /* USER CODE END CAN1_MspInit 1 */
  }
  else if(canHandle->Instance==CAN2)
  {
  /* USER CODE BEGIN CAN2_MspInit 0 */

  /* USER CODE END CAN2_MspInit 0 */
    /* CAN2 clock enable */
    __HAL_RCC_CAN2_CLK_ENABLE();
    HAL_RCC_CAN1_CLK_ENABLED++;
    if(HAL_RCC_CAN1_CLK_ENABLED==1){
      __HAL_RCC_CAN1_CLK_ENABLE();
    }

    __HAL_RCC_GPIOB_CLK_ENABLE();
    /**CAN2 GPIO Configuration
    PB5     ------> CAN2_RX
    PB6     ------> CAN2_TX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_5|GPIO_PIN_6;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF9_CAN2;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* CAN2 interrupt Init */
    HAL_NVIC_SetPriority(CAN2_TX_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(CAN2_TX_IRQn);
    HAL_NVIC_SetPriority(CAN2_RX1_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(CAN2_RX1_IRQn);
  /* USER CODE BEGIN CAN2_MspInit 1 */

  /* USER CODE END CAN2_MspInit 1 */
  }
}

void HAL_CAN_MspDeInit(CAN_HandleTypeDef* canHandle)
{

  if(canHandle->Instance==CAN1)
  {
  /* USER CODE BEGIN CAN1_MspDeInit 0 */

  /* USER CODE END CAN1_MspDeInit 0 */
    /* Peripheral clock disable */
    HAL_RCC_CAN1_CLK_ENABLED--;
    if(HAL_RCC_CAN1_CLK_ENABLED==0){
      __HAL_RCC_CAN1_CLK_DISABLE();
    }

    /**CAN1 GPIO Configuration
    PB8     ------> CAN1_RX
    PB9     ------> CAN1_TX
    */
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_8|GPIO_PIN_9);

    /* CAN1 interrupt Deinit */
    HAL_NVIC_DisableIRQ(CAN1_TX_IRQn);
    HAL_NVIC_DisableIRQ(CAN1_RX1_IRQn);
  /* USER CODE BEGIN CAN1_MspDeInit 1 */

  /* USER CODE END CAN1_MspDeInit 1 */
  }
  else if(canHandle->Instance==CAN2)
  {
  /* USER CODE BEGIN CAN2_MspDeInit 0 */

  /* USER CODE END CAN2_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_CAN2_CLK_DISABLE();
    HAL_RCC_CAN1_CLK_ENABLED--;
    if(HAL_RCC_CAN1_CLK_ENABLED==0){
      __HAL_RCC_CAN1_CLK_DISABLE();
    }

    /**CAN2 GPIO Configuration
    PB5     ------> CAN2_RX
    PB6     ------> CAN2_TX
    */
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_5|GPIO_PIN_6);

    /* CAN2 interrupt Deinit */
    HAL_NVIC_DisableIRQ(CAN2_TX_IRQn);
    HAL_NVIC_DisableIRQ(CAN2_RX1_IRQn);
  /* USER CODE BEGIN CAN2_MspDeInit 1 */

  /* USER CODE END CAN2_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */
bool setup_CAN1() {

	if (HAL_CAN_Start(&hcan1) != HAL_OK) {
		// TODO: print
		// printf("ERROR: FAILED TO START CAN1");
		return false;
	}

	queue_init(&queue_CAN, sizeof(CAN_MSG_Generic_t));
	queue_init(&queue_CAN1_Tx, sizeof(CAN_MSG_Generic_t));
	queue_init(&queue_CAN1_Rx, sizeof(CAN_MSG_Generic_t));

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

	if (HAL_CAN_ActivateNotification(&hcan1, CAN_IT_RX_FIFO0_MSG_PENDING) != HAL_OK) {
		// TODO: print
		// printf("ERROR: Failed activate CAN1 notification on RX0");
		return false;
	}

	return true;
}

bool setup_CAN2() {

	if (HAL_CAN_Start(&hcan2) != HAL_OK) {
		// TODO: print
		// printf("ERROR: FAILED TO START CAN1");
		return false;
	}

	queue_init(&queue_CAN2_Tx, sizeof(CAN_MSG_Generic_t));
	queue_init(&queue_CAN2_Rx, sizeof(CAN_MSG_Generic_t));

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

	if (HAL_CAN_ActivateNotification(&hcan2, CAN_IT_RX_FIFO0_MSG_PENDING) != HAL_OK) {
		// TODO: print
		// printf("ERROR: Failed activate CAN1 notification on RX0");
		return false;
	}

	return true;
}


HAL_StatusTypeDef send_can_msg(CAN_HandleTypeDef *hcan, CAN_TxHeaderTypeDef *pHeader, uint8_t aData[]) {
	int can_idx = 0;

	uint32_t *pTxMailbox;
	if (hcan == &hcan1) {
		pTxMailbox = &txMailbox_CAN1;
		can_idx = 0;

	} else if (hcan == &hcan2) {
		pTxMailbox = &txMailbox_CAN2;
		can_idx = 1;
	}

	// finally send CAN msg
	HAL_StatusTypeDef result = HAL_CAN_AddTxMessage(hcan, pHeader, aData, pTxMailbox);
	if (result != HAL_OK) {
		//printf("FAILED TO SEND CAN %i - e: %lu\r\n", can_idx + 1, hcan->ErrorCode);
	}
	return result;
}


void handle_CAN1_Tx_interrupt( void )
{

	__disable_irq();

	CAN_MSG_Generic_t msg;
	uint32_t txMailbox_CAN1;
	CAN_TxHeaderTypeDef Header;

	if(!queue_empty(&queue_CAN1_Tx)) // transmission queue is not empty
	{
		queue_next(&queue_CAN1_Tx, (void *)&msg); // grab first element in queue
		// setting up the header for CAN
		Header.DLC = msg.DLC;
		Header.RTR = CAN_RTR_DATA;
		Header.IDE = msg.ID_TYPE == 1 ? CAN_ID_EXT : CAN_ID_STD;
		Header.StdId = msg.ID;
		Header.ExtId = msg.ID;
		// Adding message to mailbox
		HAL_CAN_AddTxMessage((CAN_HandleTypeDef *)msg.hcan, &Header, msg.data, &txMailbox_CAN1);
	}

	__enable_irq();

}

void handle_CAN2_Tx_interrupt( void )
{

	__disable_irq();

	CAN_MSG_Generic_t msg;
	uint32_t txMailbox_CAN2;
	CAN_TxHeaderTypeDef Header;

	if(!queue_empty(&queue_CAN2_Tx)) // transmission queue is not empty
	{
		queue_next(&queue_CAN2_Tx, (void *)&msg); // grab first element in queue
		// setting up the header for CAN
		Header.DLC = msg.DLC;
		Header.RTR = CAN_RTR_DATA;
		Header.IDE = msg.ID_TYPE == 1 ? CAN_ID_EXT : CAN_ID_STD;
		Header.StdId = msg.ID;
		Header.ExtId = msg.ID;
		// Adding message to mailbox
		HAL_CAN_AddTxMessage((CAN_HandleTypeDef *)msg.hcan, &Header, msg.data, &txMailbox_CAN2);
	}

	__enable_irq();

}

void handle_CAN1_Rx_interrupt( int fifo )
{
	__disable_irq();
	CAN_MSG_Generic_t msg;
	CAN_RxHeaderTypeDef header;
	uint8_t rx_data[8];
	//int i = HAL_CAN_GetRxFifoFillLevel(hcan, fifo);
	while (HAL_CAN_GetRxFifoFillLevel(&hcan1, fifo) > 0) {
		// check if you can't recieve the message
		if (HAL_CAN_GetRxMessage(&hcan1, fifo, &header, rx_data) != HAL_OK) {
		}
		else {
			msg.hcan = &hcan1;
			msg.ID = header.IDE == CAN_ID_EXT ? header.ExtId : header.StdId;
			msg.ID_TYPE = header.IDE == CAN_ID_EXT ? 1 : 0;
			msg.DLC = header.DLC;
			msg.timestamp = HAL_GetTick();
			for (int i = 0; i < msg.DLC; i++) {
				msg.data[i] = rx_data[i];
			}

			// add message to queue
			queue_add(&queue_CAN1_Rx, &msg);

		}
	}
	__enable_irq();
}

void handle_CAN2_Rx_interrupt( int fifo )
{
	__disable_irq();
	CAN_MSG_Generic_t msg;
	CAN_RxHeaderTypeDef header;
	uint8_t rx_data[8];
	//int i = HAL_CAN_GetRxFifoFillLevel(hcan, fifo);
	while (HAL_CAN_GetRxFifoFillLevel(&hcan2, fifo) > 0) {
		// check if you can't recieve the message
		if (HAL_CAN_GetRxMessage(&hcan2, fifo, &header, rx_data) != HAL_OK) {
		}
		else {
			msg.hcan = &hcan2;
			msg.ID = header.IDE == CAN_ID_EXT ? header.ExtId : header.StdId;
			msg.ID_TYPE = header.IDE == CAN_ID_EXT ? 1 : 0;
			msg.DLC = header.DLC;
			msg.timestamp = HAL_GetTick();
			for (int i = 0; i < msg.DLC; i++) {
				msg.data[i] = rx_data[i];
			}

			// add message to queue
			queue_add(&queue_CAN1_Rx, &msg);

		}
	}
	__enable_irq();
}


/* USER CODE END 1 */
