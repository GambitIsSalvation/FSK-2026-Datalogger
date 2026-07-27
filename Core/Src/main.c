/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
#include "main.h"
#include "adc.h"
#include "can.h"
#include "dma.h"
#include "fatfs.h"
#include "i2c.h"
#include "iwdg.h"
#include "rtc.h"
#include "spi.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>
#include "gps.h"
#include "ringbuffer.h"
#include "MPU9250.h"
#include "VehicleState.h"
#include "DisplayTransmit.h"
#include "CrossDetect.h"
#include "APPS.h"
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

// GPS //
extern GPS_t GPS; 			// GPS Data

int retry_count = 0;

float GPS_prev_lat = 0;
float GPS_prev_lon = 0;


// MPU //
MPU9250_t myMPU;
double gyro_bias[3] = {0, 0, 0};
double mag_bias[3] = {0, 0, 0};
double cal_gyro[3];
double cal_mag[3];
static uint32_t imu_prev_tick = 0;
static uint32_t mag_prev_tick = 0;
static uint32_t sd_prev_tick = 0;
static uint32_t sync_prev_tick = 0;
volatile uint8_t sync_pending = 0;


// FATFS & microSD //
char sd_temp[2048];
uint16_t current_len = 0; // SDIO 용 데이터 길이 확인용
uint16_t remain = 0;
uint8_t can_count = 0;

__attribute__((aligned(4))) FATFS   fs;   // FATFS structure
FIL     file; // File object structure
FRESULT res;  // File function return code
UINT    bytes_written; // The number of bytes actually written
uint32_t sync_count = 0;

// Display and States
VehicleState VS;
DisplayTransmit DISP;
extern uint8_t disp_dma_ready;
extern uint8_t laps;
extern uint32_t lap_time;
extern uint32_t pre_tick;

// RTC
RTC_DateTypeDef sDate;
RTC_TimeTypeDef sTime;

volatile uint8_t current_Hours = 0;
volatile uint8_t current_Minutes = 0;
volatile uint8_t current_Seconds = 0;
volatile int current_SubSeconds = 0;

uint16_t sync_error = 0;

uint32_t UTC_temp  = 0;
volatile uint8_t UTC_flag = 0;

// Module Status
uint8_t init_status = 0; // 0:CAN  1:SD  2:GPS  3:IMU 4: UTC
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

// SPI2 통신 기본 PFP //
void SD_Init(void);
void SD_WriteFile(char *data, int len);
void SD_SaveFile(void);

// UART 통신 기본 PFP //
int _write(int file, char *ptr, int len);   // printf UART 출력
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

int _write(int file, char *ptr, int len) // printf UART 출력
{
    HAL_UART_Transmit(&huart2, (uint8_t*)ptr, len, HAL_MAX_DELAY);
    return len;
}

// SDIO //
void SD_Init(void) {
	// 1. SD card Mount
	res = f_mount(&fs, "", 1);
	if (res != FR_OK) {
		init_status |= (1 << 1);
	}

	res = f_open(&file, "data.csv", FA_WRITE | FA_OPEN_APPEND);
	if (res != FR_OK) {
		init_status |= (1 << 1);
	}
}

void SD_WriteFile(char *data, int len){
	res = f_write(&file, data, len, &bytes_written);
	if (res == FR_OK) {
		//printf("Write Complete!\n");
	} else {
		//printf("Write Failed! Error Code : %d\n", res);
	}
}

void SD_SaveFile(void){
	res = f_sync(&file);
	if (res == FR_OK) {
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
		//printf("Sync Complete!\n");
	} else {
		//printf("Sync Failed! Error Code : %d\n", res);
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
		Error_Handler();
	}
}

// RTC //
void get_time(void)
{
	HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BCD);
	HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BCD);
}

void CAN_Send_Blocking(CAN_HandleTypeDef *hcan, uint32_t id, uint8_t *data, uint8_t len)
{
	HAL_Delay(10);
    uint32_t timeout = HAL_GetTick();
    // 빈 메일박스가 생길 때까지 대기
    while (HAL_CAN_GetTxMailboxesFreeLevel(hcan) == 0) {
        if (HAL_GetTick() - timeout > 10) return;   // 10ms 타임아웃
    }
    CAN_Send(hcan, id, data, len);
}
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
  MX_USART2_UART_Init();
  MX_CAN1_Init();
  MX_RTC_Init();
  MX_FATFS_Init();
  MX_USART3_UART_Init();
  MX_I2C1_Init();
  MX_SPI2_Init();
  MX_ADC1_Init();
  MX_USART6_UART_Init();
  MX_IWDG_Init();
  /* USER CODE BEGIN 2 */
  HAL_Delay(250);

  // CAN Init
  CAN_Config();

  // SD Init
  SD_Init();

  // GPS Init
  GPS_Init();

  // MPU Init
  MPU_Init(&myMPU);

  // Logging Start
  get_time();
  float subsec = (float)(255 - sTime.SubSeconds) / 256.0f;

  current_len += snprintf(sd_temp + current_len, sizeof(sd_temp) - current_len,
		  "%02x:%02x:%02x.%03d, SYSTEM, CONFIG ON, %d\n",
		  sTime.Hours, sTime.Minutes, sTime.Seconds, (int)(subsec*1000), init_status);

  DisplayTransmit_Init(&DISP, &VS);


  //모컨 통신속도 제어
  /*
  uint8_t dat0[3] ={0x3D, 0x32, 0x0A};
  uint8_t dat1[3] ={0x3D, 0x30, 0x0A};
  uint8_t dat2[3] ={0x3D, 0x22, 0x64};
  uint8_t dat3[3] ={0x3D, 0x27, 0x64};
  uint8_t dat4[3] ={0x3D, 0x8A, 0x64};
  uint8_t dat5[3] ={0x3D, 0x66, 0x64};
  uint8_t dat6[3] ={0x3D, 0x49, 0x64};
  uint8_t dat7[3] ={0x3D, 0x4A, 0x64};
  CAN_Send_Blocking(&hcan1, MOTOTX, dat0, 3);
  CAN_Send_Blocking(&hcan1, MOTOTX, dat1, 3);
  CAN_Send_Blocking(&hcan1, MOTOTX, dat2, 3);
  CAN_Send_Blocking(&hcan1, MOTOTX, dat3, 3);
  CAN_Send_Blocking(&hcan1, MOTOTX, dat4, 3);
  CAN_Send_Blocking(&hcan1, MOTOTX, dat5, 3);
  CAN_Send_Blocking(&hcan1, MOTOTX, dat6, 3);
  CAN_Send_Blocking(&hcan1, MOTOTX, dat7, 3);
  */
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  HAL_IWDG_Refresh(&hiwdg);

	  // UTC - RTC 동기화
	  if(UTC_flag) {
		  UTC_temp = GPS.utc_time;
		  sTime.Hours = RTC_ByteToBcd2(UTC_temp / 10000 + 9);
		  sTime.Minutes = RTC_ByteToBcd2((UTC_temp / 100) % 100);
		  sTime.Seconds = RTC_ByteToBcd2(UTC_temp % 100 + 1);
		  sTime.SubSeconds = 0;
		  if(HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BCD) == HAL_OK) { UTC_flag = 0; }
		  if(!init_status) {
			  current_len += snprintf(sd_temp + current_len, sizeof(sd_temp) - current_len,
					  "%02x:%02x:%02x.%03d, SYSTEM, RTC-UTC SYNC\n",
					  sTime.Hours, sTime.Minutes, sTime.Seconds, (int)(subsec*1000));
			  init_status |= 1<<4;
		  }
	  }

	  // CAN Messages
	  can_count = 0;
	  while(IsPopAvailable() && can_count < 32)
	  {
		  	  get_time();
		  	  float subsec = (float)(255 - sTime.SubSeconds) / 256.0f;
	          RxMessage received = RingPop();
		  	  Classifier(&VS, received);

		  	  remain = sizeof(sd_temp) - current_len;
		  	  if (remain < 128) break;

		  	  current_len += snprintf(sd_temp + current_len, sizeof(sd_temp) - current_len, "%02x:%02x:%02x.%03d",
		  			sTime.Hours, sTime.Minutes, sTime.Seconds, (int)(subsec*1000));

	          current_len += snprintf(sd_temp + current_len, sizeof(sd_temp) - current_len, ", %lx", received.RxHeader.StdId);
	          for(int i = 0; i < received.RxHeader.DLC; i++) {

	        	  current_len += snprintf(sd_temp + current_len, sizeof(sd_temp) - current_len, ", %d\n", received.RxData[i]);
	          }
	          can_count++;
	  }

	  // GPS (10Hz)
	  remain = sizeof(sd_temp) - current_len;
	  if (remain >= 128)
	  {
		  if(GPS.dec_latitude != GPS_prev_lat || GPS.dec_longitude != GPS_prev_lon)
		  {
			  get_time();
			  float subsec = (float)(255 - sTime.SubSeconds) / 256.0f;

			  current_len += snprintf(sd_temp + current_len, sizeof(sd_temp) - current_len, "%02x:%02x:%02x.%03d",
			  		  			sTime.Hours, sTime.Minutes, sTime.Seconds, (int)(subsec*1000));

			  current_len += snprintf(sd_temp + current_len, sizeof(sd_temp) - current_len, ", GPS, %f, %f\n",
					  GPS.dec_latitude,
					  GPS.dec_longitude);

			  //printf("GPS : %s", sd_temp);

			  if(get_line_intersection(GPS_prev_lat, GPS_prev_lon,
					  	  	  	  GPS.dec_latitude, GPS.dec_longitude))
			  {
				  current_len += snprintf(sd_temp + current_len, sizeof(sd_temp) - current_len, "%02x:%02x:%02x.%03d",
				  			  		  			sTime.Hours, sTime.Minutes, sTime.Seconds, (int)(subsec*1000));
				  current_len += snprintf(sd_temp + current_len, sizeof(sd_temp) - current_len, ", LAP, %d, %u\n",
						  	  	  	  	  laps, lap_time);
			  }

			  // 8. GPS 데이터 임시 저장 후 비교
			  GPS_prev_lat = GPS.dec_latitude;
			  GPS_prev_lon = GPS.dec_longitude;
		  }
	  }

	  // Laps
	  VS.laps = laps;
	  VS.lap_time = (uint32_t)(HAL_GetTick() - pre_tick);


	  // IMU (200Hz)
	  remain = sizeof(sd_temp) - current_len;
	  if (remain >= 128) {
		  if(HAL_GetTick() - imu_prev_tick >= 5) {
			  // 0. IMU 데이터 읽기
			  imu_prev_tick = HAL_GetTick();

			  MPU9250_ReadAcc(&myMPU);
			  MPU9250_ReadGyro(&myMPU);

			  cal_gyro[0] = myMPU.gyro[0] - gyro_bias[0];
			  cal_gyro[1] = myMPU.gyro[1] - gyro_bias[1];
			  cal_gyro[2] = myMPU.gyro[2] - gyro_bias[2];

			  get_time();
			  float subsec = (float)(255 - sTime.SubSeconds) / 256.0f;

			  current_len += snprintf(sd_temp + current_len, sizeof(sd_temp) - current_len, "%02x:%02x:%02x.%03d",
			  		  			sTime.Hours, sTime.Minutes, sTime.Seconds, (int)(subsec*1000));


			  current_len += snprintf(sd_temp + current_len, sizeof(sd_temp) - current_len,
					  ", IMU, %f, %f, %f, %f, %f, %f\n",
					  myMPU.acc[0], myMPU.acc[1], myMPU.acc[2],
					  cal_gyro[0], cal_gyro[1], cal_gyro[2]);

			  VehicleState_set_lat_g(&VS,  (uint16_t)(myMPU.acc[0] * 10.0f + 30.0f));
			  VehicleState_set_long_g(&VS, (uint16_t)(myMPU.acc[1] * 10.0f + 30.0f));
		  }
	  }

	  // MAGNETOMETER (100Hz)
	  remain = sizeof(sd_temp) - current_len;
	  if (remain >= 128) {
	  	if(HAL_GetTick() - mag_prev_tick >= 10) {
	  		mag_prev_tick = HAL_GetTick();

	  	    MPU9250_ReadMag(&myMPU);

	  		cal_mag[0] = myMPU.mag[0] - mag_bias[0];
	  		cal_mag[1] = myMPU.mag[1] - mag_bias[1];
	  		cal_mag[2] = myMPU.mag[2] - mag_bias[2];


	  		get_time();
	  		float subsec = (float)(255 - sTime.SubSeconds) / 256.0f;

	  		current_len += snprintf(sd_temp + current_len, sizeof(sd_temp) - current_len, "%02x:%02x:%02x.%03d",
	  			  		  			sTime.Hours, sTime.Minutes, sTime.Seconds, (int)(subsec*1000));


	  		current_len += snprintf(sd_temp + current_len, sizeof(sd_temp) - current_len,", MAG, %f, %f, %f\n",
	  					  	  	  	cal_mag[0], cal_mag[1], cal_mag[2]);
	    }
	  }

	  // SD카드 동기롸
	  if (current_len >= 512 || HAL_GetTick() - sd_prev_tick >= 1000)
	  {
		  if(current_len > 0) {
			  SD_WriteFile(sd_temp, current_len);
			  current_len = 0;
			  sync_pending = 1;
			  sd_prev_tick = HAL_GetTick();
		  }

		  if (HAL_GetTick() - sync_prev_tick >= 5000) {
		      if (sync_pending) {
		          SD_SaveFile();
		          sync_pending = 0;
		          sync_count++;
		      }
		      sync_prev_tick = HAL_GetTick();
		  }
	  }

	  // APPS 판정
	  APPS();
	  VS.apps = get_p();

	  DisplayTransmit_Transmit(&DISP);
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
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

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_LSI
                              |RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 180;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 8;
  RCC_OscInitStruct.PLL.PLLR = 2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Activate the Over-Drive mode
  */
  if (HAL_PWREx_EnableOverDrive() != HAL_OK)
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

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
/* CallBack Function */
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart) {
    if (huart->Instance == USART6) {   // 디스플레이 UART인지 확인
        disp_dma_ready = 1;
    }
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
    if (huart->Instance == USART3) {
        GPS_UART_CallBack();
    }
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_PIN) {
	if(GPIO_PIN == GPS_PPS_Pin) {
		UTC_flag = 1;
	}
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
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
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
