/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    can.h
  * @brief   This file contains all the function prototypes for
  *          the can.c file
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
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __CAN_H__
#define __CAN_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */
#include <stdbool.h>
#include <QUTMS_can.h>
#include <CAN_VCU.h>
#include "queue.h"
/* USER CODE END Includes */

extern CAN_HandleTypeDef hcan1;

extern CAN_HandleTypeDef hcan2;

/* USER CODE BEGIN Private defines */

#define CAN_QUEUE_SIZE 50
extern message_queue_t queue_CAN1_Tx;
extern message_queue_t queue_CAN2_Tx;
extern message_queue_t queue_CAN1_Rx;
extern message_queue_t queue_CAN2_Rx;
extern uint32_t can1Mailbox;
extern uint32_t can2Mailbox;

/* USER CODE END Private defines */

void MX_CAN1_Init(void);
void MX_CAN2_Init(void);

/* USER CODE BEGIN Prototypes */
bool setup_CAN1();
bool setup_CAN2();
HAL_StatusTypeDef send_can_msg(CAN_HandleTypeDef *hcan, CAN_TxHeaderTypeDef *pHeader, uint8_t aData[]);
void handle_CAN1_Tx_interrupt( void );
void handle_CAN1_Rx_interrupt( int fifo );
void handle_CAN2_Tx_interrupt( void );
void handle_CAN2_Rx_interrupt( int fifo );
/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __CAN_H__ */

