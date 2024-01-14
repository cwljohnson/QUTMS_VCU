/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f2xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */
/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define PROFET_SENSE_ADC_Pin GPIO_PIN_0
#define PROFET_SENSE_ADC_GPIO_Port GPIOA
#define DIV_5V_ADC_Pin GPIO_PIN_2
#define DIV_5V_ADC_GPIO_Port GPIOA
#define DIV_12V_ADC_Pin GPIO_PIN_3
#define DIV_12V_ADC_GPIO_Port GPIOA
#define ADC_RST_Pin GPIO_PIN_4
#define ADC_RST_GPIO_Port GPIOC
#define ADC_REFSEL_Pin GPIO_PIN_5
#define ADC_REFSEL_GPIO_Port GPIOC
#define ADC_ALARM_Pin GPIO_PIN_0
#define ADC_ALARM_GPIO_Port GPIOB
#define ADC_CS_Pin GPIO_PIN_1
#define ADC_CS_GPIO_Port GPIOB
#define PROFET_IN1_Pin GPIO_PIN_7
#define PROFET_IN1_GPIO_Port GPIOE
#define PROFET_DSEL_Pin GPIO_PIN_8
#define PROFET_DSEL_GPIO_Port GPIOE
#define PROFET_DEN_Pin GPIO_PIN_9
#define PROFET_DEN_GPIO_Port GPIOE
#define PROFET_IN0_Pin GPIO_PIN_10
#define PROFET_IN0_GPIO_Port GPIOE
#define ISRC_3_CS_Pin GPIO_PIN_11
#define ISRC_3_CS_GPIO_Port GPIOE
#define ISRC_4_CS_Pin GPIO_PIN_12
#define ISRC_4_CS_GPIO_Port GPIOE
#define ISRC_1_CS_Pin GPIO_PIN_13
#define ISRC_1_CS_GPIO_Port GPIOE
#define ISRC_2_CS_Pin GPIO_PIN_14
#define ISRC_2_CS_GPIO_Port GPIOE
#define IMU_INT_Pin GPIO_PIN_11
#define IMU_INT_GPIO_Port GPIOB
#define IMU_CS_Pin GPIO_PIN_12
#define IMU_CS_GPIO_Port GPIOB
#define SDP_Pin GPIO_PIN_8
#define SDP_GPIO_Port GPIOD
#define SC_Pin GPIO_PIN_9
#define SC_GPIO_Port GPIOD
#define SD_Pin GPIO_PIN_10
#define SD_GPIO_Port GPIOD
#define SE_Pin GPIO_PIN_11
#define SE_GPIO_Port GPIOD
#define SB_Pin GPIO_PIN_15
#define SB_GPIO_Port GPIOD
#define SA_Pin GPIO_PIN_6
#define SA_GPIO_Port GPIOC
#define SF_Pin GPIO_PIN_7
#define SF_GPIO_Port GPIOC
#define SG_Pin GPIO_PIN_8
#define SG_GPIO_Port GPIOC
#define PMOD_CS_Pin GPIO_PIN_0
#define PMOD_CS_GPIO_Port GPIOD
#define PMOD_INT_Pin GPIO_PIN_1
#define PMOD_INT_GPIO_Port GPIOD
#define PMOD_RST_Pin GPIO_PIN_2
#define PMOD_RST_GPIO_Port GPIOD
#define PMOD_IO7_Pin GPIO_PIN_3
#define PMOD_IO7_GPIO_Port GPIOD
#define PMOD_IO8_Pin GPIO_PIN_4
#define PMOD_IO8_GPIO_Port GPIOD
#define BOOT_CTRL_Pin GPIO_PIN_7
#define BOOT_CTRL_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
