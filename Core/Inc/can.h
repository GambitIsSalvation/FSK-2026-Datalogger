/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    can.h
  * @brief   This file contains all the function prototypes for
  *          the can.c file
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
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

/* USER CODE END Includes */

extern CAN_HandleTypeDef hcan1;

/* USER CODE BEGIN Private defines */
// CAN ID
#define MOTORX  0x181 // 모터 컨트롤러 수신 ID
#define MOTOTX  0x201 // 모터 컨트롤러 송신 ID
#define BMSRX_1 0x6b1 // BMS 수신 ID_1
#define BMSRX_2 0x6b0 // BMS Error 수신 ID

// 모터 컨트롤러 REGID
#define RPM_COMMAND 0x32 // 타겟 RPM
#define RPM_ACTUAL  0x30 // 현재 RPM
#define IQ_COMMAND  0x22 // 타겟 유효전류
#define IQ_ACTUAL   0x27 // 현재 유효전류
#define V_OUT       0x8A // 인버터 -> 모터 전압
#define V_IN        0x66 // 배터리 -> 인버터 전압
#define T_CTRL      0x4A // 컨트롤러 온도
#define T_MOTOR     0x49 // 모터 온도

#define CHECKSUM_FALSE  ((uint8_t)0x00)
#define CHECKSUM_TRUE   ((uint8_t)0x01)

typedef struct {
	CAN_RxHeaderTypeDef RxHeader;
	uint8_t RxData[8];
} RxMessage; // 모터 컨트롤러 메세지 저장 구조체

/* USER CODE END Private defines */

void MX_CAN1_Init(void);

/* USER CODE BEGIN Prototypes */

void CAN_Config(void);                                                                      // CAN 필터 설정
void CAN_Send(CAN_HandleTypeDef *hcan, uint32_t id, uint8_t *data, uint8_t length);         // CAN 데이터 프레임 송신
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan);                            // CAN Rx 인터럽트 콜백 함수

/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __CAN_H__ */

