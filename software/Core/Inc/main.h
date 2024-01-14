/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
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
#include <stdint.h>

#include <CAN_VCU.h>
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

#define MAX_VCU 16

#define VCU_CURRENT_ID VCU_ID_DASH


#define HEARTBEAT_TIMEOUT 1000U

// 100ms
#define PERIPHERAL_RETRY 100U

// 1000ms
#define PERIPHERAL_TIMEOUT 1000U

// 500ms
#define SENSOR_RETRY 500U

// 5000ms
#define SENSOR_TIMEOUT 5000U

// 1000ms
#define HEARTBEAT_PRINT_TIME 1000U

// 100ms
#define SENSOR_IMPLAUSIBILITY_TIMEOUT 100U

// 500ms
#define STEERING_IMPLAUSIBILTIY_TIMEOUT 500U

// disable shutdown latching
#define DASH_DEBUG_SHDN 0

#define DASH_LATCH_BYPASS_TIME 3000

#define DEBUG_BMU 0
#define DEBUG_INV 0
#define DEBUG_MCISO 0
#define RTD_DEBUG 0
#define DEBUG_HB_FAIL 1

#define DRIVERLESS_CTRL 1

#define DEBUG_EBS_ASMS_CHECK 0
#define DEBUG_EBS_BRAKE_CHECK 1

// inverter settings

#define INV_MAX_CURRENT 120
#define INV_DEADZONE_MIN 400
#define INV_DEADZONE_MAX 500

#define INV_REGEN_ENABLE 0
#define INV_REGEN_KMH_CUTOFF 10
#define INV_REGEN_MAX_CURRENT 60

#define INV_TV_ENABLE 0
#define INV_TV_DEADZONE 10
#define INV_TV_SCALAR 0
#define INV_TV_BOOST 0

#define WHEEL_RADIUS 0.4064

extern uint8_t VCU_ID;
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

//#define SW0_ALT_Pin GPIO_PIN_0
//#define SW0_ALT_GPIO_Port GPIOA

#define LED_REG_IMD DS4424_REG_OUT_0
#define LED_REG_BMU DS4424_REG_OUT_1
#define LED_REG_PDOC DS4424_REG_OUT_3

// 2 seconds
#define BMU_HEARTBEAT_TIMEOUT 2000

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
