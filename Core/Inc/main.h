/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
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
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */
extern uint8_t init_status;
/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define B1_Pin GPIO_PIN_13
#define B1_GPIO_Port GPIOC
#define B1_EXTI_IRQn EXTI15_10_IRQn
#define SD_MOSI_Pin GPIO_PIN_1
#define SD_MOSI_GPIO_Port GPIOC
#define SD_MISO_Pin GPIO_PIN_2
#define SD_MISO_GPIO_Port GPIOC
#define APPS1_Pin GPIO_PIN_0
#define APPS1_GPIO_Port GPIOA
#define SERIAL_TX_Pin GPIO_PIN_2
#define SERIAL_TX_GPIO_Port GPIOA
#define SERIAL_RX_Pin GPIO_PIN_3
#define SERIAL_RX_GPIO_Port GPIOA
#define APPS_OUT_Pin GPIO_PIN_4
#define APPS_OUT_GPIO_Port GPIOA
#define LD2_Pin GPIO_PIN_5
#define LD2_GPIO_Port GPIOA
#define LINEAR_FR_Pin GPIO_PIN_6
#define LINEAR_FR_GPIO_Port GPIOA
#define LINEAR_FL_Pin GPIO_PIN_7
#define LINEAR_FL_GPIO_Port GPIOA
#define GPS_RX_Pin GPIO_PIN_5
#define GPS_RX_GPIO_Port GPIOC
#define SD_SCK_Pin GPIO_PIN_13
#define SD_SCK_GPIO_Port GPIOB
#define DISPLAY_TX_Pin GPIO_PIN_6
#define DISPLAY_TX_GPIO_Port GPIOC
#define DISPLAY_RX_Pin GPIO_PIN_7
#define DISPLAY_RX_GPIO_Port GPIOC
#define GPS_PPS_Pin GPIO_PIN_8
#define GPS_PPS_GPIO_Port GPIOC
#define GPS_PPS_EXTI_IRQn EXTI9_5_IRQn
#define SD_CS_Pin GPIO_PIN_8
#define SD_CS_GPIO_Port GPIOA
#define TMS_Pin GPIO_PIN_13
#define TMS_GPIO_Port GPIOA
#define TCK_Pin GPIO_PIN_14
#define TCK_GPIO_Port GPIOA
#define GPS_TX_Pin GPIO_PIN_10
#define GPS_TX_GPIO_Port GPIOC
#define BRAKE_5V_Pin GPIO_PIN_11
#define BRAKE_5V_GPIO_Port GPIOC
#define RTD_LED_5V_Pin GPIO_PIN_2
#define RTD_LED_5V_GPIO_Port GPIOD
#define SWO_Pin GPIO_PIN_3
#define SWO_GPIO_Port GPIOB
#define IMU_SCL_Pin GPIO_PIN_6
#define IMU_SCL_GPIO_Port GPIOB
#define IMU_SDA_Pin GPIO_PIN_7
#define IMU_SDA_GPIO_Port GPIOB
#define SD_ON_Pin GPIO_PIN_8
#define SD_ON_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
