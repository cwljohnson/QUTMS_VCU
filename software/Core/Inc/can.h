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
#include <queue.h>
/* USER CODE END Includes */

extern CAN_HandleTypeDef hcan1;

extern CAN_HandleTypeDef hcan2;

/* USER CODE BEGIN Private defines */
#define CAN_QUEUE_SIZE 50

extern message_queue_t queue_CAN;
extern message_queue_t queue_CAN_OD;

#if VCU_CURRENT_ID == VCU_ID_CTRL
extern message_queue_t queue_CAN_steering;
#endif

extern uint32_t txMailbox_CAN1;
extern uint32_t txMailbox_CAN2;
/* USER CODE END Private defines */

void MX_CAN1_Init(void);
void MX_CAN2_Init(void);

/* USER CODE BEGIN Prototypes */
bool setup_CAN();
bool init_CAN1();
bool init_CAN2();
HAL_StatusTypeDef send_can_msg(CAN_HandleTypeDef *hcan, CAN_TxHeaderTypeDef *pHeader, uint8_t aData[]);

void can_rx_interrupt(CAN_HandleTypeDef *hcan, int fifo);
void can_tx_interrupt(CAN_HandleTypeDef *hcan);
/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __CAN_H__ */

