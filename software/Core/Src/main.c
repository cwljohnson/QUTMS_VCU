/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "can.h"
#include "dma.h"
#include "i2c.h"
#include "spi.h"
#include "tim.h"
#include "usb_device.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <ads8668.h>

#include "12vSW.h"

#include <qutms_can.h>
#include <CAN_VCU.h>
#include <CAN_BMU.h>
#include <CAN_SHDN.h>
#include <window_filtering.h>

#include "imu.h"
#include "MPXV7002.h"
#include "VCU_shutdown.h"

#include <FSM.h>
#include "heartbeat.h"
#include "states.h"
#include "debugCAN.h"
#include "can_dict.h"

#include "p_imu.h"
#include "p_adc.h"
#include "p_watchdog.h"

#include "s_pedalBox.h"
#include "s_suspensionTravel.h"
#include "s_shutdown.h"
#include "s_steeringAngle.h"
#include "rtd.h"

#include "7Seg.h"
#include "EBS_ADC.h"
#include "max5548.h"

#include "dvl_emergency.h"



//#include "usbd_cdc_if.h"
#include <unistd.h>
#include "s_ctrl_steering.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
fsm_t fsm;
uint8_t VCU_ID = VCU_CURRENT_ID;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
void fsm_log_uart(const char *data, size_t len);
void fsm_state_enter(fsm_t *fsm_def);
void fsm_state_exit(fsm_t *fsm_def);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

int _write(int file, char *data, int len) {
	if ((file != STDOUT_FILENO) && (file != STDERR_FILENO)) {
		return -1;
	}

	//CDC_Transmit_FS((uint8_t*) data, len);

	/*
	 // log whatever we transmit to serial
	 serial_log_t log_wrapper;
	 memcpy(log_wrapper.data, data, len);
	 log_wrapper.len = len;
	 queue_add(&queue_serial_log, &log_wrapper);
	 */
	return len;
}
/*
 __weak void HAL_WWDG_EarlyWakeupCallback(WWDG_HandleTypeDef *hwwdg) {
 // Prevent unused argument(s) compilation warning
 UNUSED(hwwdg);

 /* NOTE: This function should not be modified, when the callback is needed,
 the HAL_WWDG_EarlyWakeupCallback could be implemented in the user file

 set_7seg(VCU_CURRENT_ID, true);
 }
 */
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_CAN1_Init();
  MX_ADC1_Init();
  MX_ADC2_Init();
  MX_CAN2_Init();
  MX_I2C3_Init();
  MX_SPI1_Init();
  MX_SPI2_Init();
  MX_USB_DEVICE_Init();
  MX_TIM2_Init();
  /* USER CODE BEGIN 2 */

	// init fsm
	fsm.log = &fsm_log_uart;
	fsm_init(&fsm, &state_start, &fsm_state_enter, &fsm_state_exit);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */

	// display current VCU_ID
	set_7seg(VCU_CURRENT_ID, false);
	setup_CAN();
	while (1) {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
		// update key timers
		timer_update(&timer_watchdog, NULL);
		timer_update(&timer_adc, NULL);
		timer_update(&timer_heartbeat, NULL);
		timer_update(&timer_OD, NULL);
		timer_update(&timer_IMU, NULL);

#if VCU_CURRENT_ID == VCU_ID_SHDN
		timer_update(&timer_shdn_status, NULL);
#endif

#if VCU_CURRENT_ID == VCU_ID_CTRL
		timer_update(&timer_pedal_adc, NULL);
		timer_update(&timer_rtd_siren, NULL);
		timer_update(&timer_horn, NULL);
		timer_update(&timer_ctrl_steering, NULL);

#if DRIVERLESS_CTRL == 1
		timer_update(&timer_dvl_emergency, NULL);
#endif

#endif

#if VCU_CURRENT_ID == VCU_ID_DASH
		timer_update(&timer_steering, NULL);
#endif

#if (VCU_CURRENT_ID == VCU_ID_DASH) || (VCU_CURRENT_ID == VCU_ID_SHDN)
		timer_update(&timer_suspension_adc, NULL);
#endif

#if VCU_CURRENT_ID == VCU_ID_EBS_ADC
		timer_update(&timer_ebs_adc_check, NULL);
#endif

		// continue fsm
		fsm_iterate(&fsm);
	}
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 16;
  RCC_OscInitStruct.PLL.PLLN = 192;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 5;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
void fsm_state_enter(fsm_t *fsm_def) {
	VCU_heartbeatState.stateID = fsm_def->currentState->stateID;
	debugCAN_enterState(fsm_def->currentState->stateID);
	heartbeat_timer_cb(NULL);
}

void fsm_state_exit(fsm_t *fsm_def) {
	debugCAN_exitState(fsm_def->currentState->stateID);
	heartbeat_timer_cb(NULL);
}

void fsm_log_uart(const char *data, size_t len) {
	//CDC_Transmit_FS(data, len);
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */
	//__disable_irq();
	while (1) {
	}
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
