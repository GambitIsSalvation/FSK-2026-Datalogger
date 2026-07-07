/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    can.c
  * @brief   This file provides code for the configuration
  *          of the CAN instances.
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
/* Includes ------------------------------------------------------------------*/
#include "can.h"

/* USER CODE BEGIN 0 */

// ID는 MCU를 기준으로 함.

CAN_TxHeaderTypeDef TxHeader; // 송신 헤더 정의
CAN_RxHeaderTypeDef RxHeader; // 수신 헤더 정의

uint8_t TxData[8]; // 송신 데이터 메모리 할당
uint8_t RxData[8]; // 수신 데이터 메모리 할당

RxMessage RxMsg; // 모터 컨트롤러 메세지 구조체 정의

uint32_t TxMailbox; // 송신 메일박스 메모리 할당
/* USER CODE END 0 */

CAN_HandleTypeDef hcan1;

/* CAN1 init function */
void MX_CAN1_Init(void)
{

  /* USER CODE BEGIN CAN1_Init 0 */

  /* USER CODE END CAN1_Init 0 */

  /* USER CODE BEGIN CAN1_Init 1 */

  /* USER CODE END CAN1_Init 1 */
  hcan1.Instance = CAN1;
  hcan1.Init.Prescaler = 5;
  hcan1.Init.Mode = CAN_MODE_NORMAL;
  hcan1.Init.SyncJumpWidth = CAN_SJW_1TQ;
  hcan1.Init.TimeSeg1 = CAN_BS1_15TQ;
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

void HAL_CAN_MspInit(CAN_HandleTypeDef* canHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(canHandle->Instance==CAN1)
  {
  /* USER CODE BEGIN CAN1_MspInit 0 */

  /* USER CODE END CAN1_MspInit 0 */
    /* CAN1 clock enable */
    __HAL_RCC_CAN1_CLK_ENABLE();

    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**CAN1 GPIO Configuration
    PA11     ------> CAN1_RX
    PA12     ------> CAN1_TX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_11|GPIO_PIN_12;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF9_CAN1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* CAN1 interrupt Init */
    HAL_NVIC_SetPriority(CAN1_RX0_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(CAN1_RX0_IRQn);
    HAL_NVIC_SetPriority(CAN1_SCE_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(CAN1_SCE_IRQn);
  /* USER CODE BEGIN CAN1_MspInit 1 */

  /* USER CODE END CAN1_MspInit 1 */
  }
}

void HAL_CAN_MspDeInit(CAN_HandleTypeDef* canHandle)
{

  if(canHandle->Instance==CAN1)
  {
  /* USER CODE BEGIN CAN1_MspDeInit 0 */

  /* USER CODE END CAN1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_CAN1_CLK_DISABLE();

    /**CAN1 GPIO Configuration
    PA11     ------> CAN1_RX
    PA12     ------> CAN1_TX
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_11|GPIO_PIN_12);

    /* CAN1 interrupt Deinit */
    HAL_NVIC_DisableIRQ(CAN1_RX0_IRQn);
    HAL_NVIC_DisableIRQ(CAN1_SCE_IRQn);
  /* USER CODE BEGIN CAN1_MspDeInit 1 */

  /* USER CODE END CAN1_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */
void CAN_Config(void) // CAN 필터 설정
{
	// 1. 모터 컨트롤러 필터 //
	CAN_FilterTypeDef MotoFilter;

	MotoFilter.FilterBank = 0;
	MotoFilter.FilterMode = CAN_FILTERMODE_IDMASK;
	MotoFilter.FilterScale = CAN_FILTERSCALE_32BIT;
	//MotoFilter.FilterIdHigh = (MotoRx << 5);
	MotoFilter.FilterIdHigh = 0x0000;
	MotoFilter.FilterIdLow = 0x0000;
	//MotoFilter.FilterMaskIdHigh = (MotoRx << 5);
	MotoFilter.FilterMaskIdHigh = 0x0000;
	MotoFilter.FilterMaskIdLow = 0x0000;
	MotoFilter.FilterFIFOAssignment = CAN_RX_FIFO0;
	MotoFilter.FilterActivation = ENABLE;
	MotoFilter.SlaveStartFilterBank = 14;

    if (HAL_CAN_ConfigFilter(&hcan1, &MotoFilter) != HAL_OK)
    {
    	printf("Motor Controller filter setup failed!\n");
    }

    if(HAL_CAN_Start(&hcan1) != HAL_OK )
    {
    	printf("CAN1 start failed! Error Code : %d\n", hcan1.ErrorCode);
    	Error_Handler();
    }

    if (HAL_CAN_ActivateNotification(&hcan1, CAN_IT_RX_FIFO0_MSG_PENDING) != HAL_OK)
    {
        printf("CAN1 Interrupt enable failed!");
    }

    //..
}

void CAN_Send(CAN_HandleTypeDef *hcan, uint32_t id, uint8_t *data, uint8_t length) // CAN 데이터 프레임 송신
{
	TxHeader.StdId = id;
	TxHeader.ExtId = 0;
	TxHeader.RTR = CAN_RTR_DATA;
	TxHeader.DLC = length;
	TxHeader.IDE = CAN_ID_STD;
	TxHeader.TransmitGlobalTime = DISABLE;

    if (HAL_CAN_AddTxMessage(hcan, &TxHeader, data, &TxMailbox) != HAL_OK)
    {
        printf("CAN Send Error!\n");
    }
}

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan) // CAN Rx 인터럽트 콜백 함수
{
	if(HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &RxHeader, RxData) == HAL_OK)
	{
		if (hcan == &hcan1)
		{
			if(IsWriteAvailable()){                                 // Push가 가능한 경우
				RxMsg.RxHeader = RxHeader;                          // 헤더 패키징
				memcpy(RxMsg.RxData, RxData, RxHeader.DLC);         // 데이터 패키징
				RingPush(RxMsg);                                    // 링버퍼에 Push
			}
			// 생각해야 할 부분이 여기서는 이미 FIFO에서 패킷을 풀어둔 상태라
			// 링 버퍼가 가득 찼을때 데이터가 완전 소실될 가능성이 있음
		}
	}
	else
	{
		printf("Fail to Receive CAN Messages!\n");
	}
}
/* USER CODE END 1 */
