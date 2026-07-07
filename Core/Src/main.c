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
uint8_t ubx_rate_5hz[] =    // Changing GPS Update Rate Protocol
{
    0xB5, 0x62,       		// Header
    0x06, 0x08,       		// Class(CFG), ID(RATE)
    0x06, 0x00,       		// Length (6 bytes)
    0xC8, 0x00,       		// measRate (200ms = 5Hz) (Little Endian)
    0x01, 0x00,       		// navRate (1 cycle)
    0x01, 0x00,       		// timeRef (GPS Time)
    0xDE, 0x6A        		// Checksum (CK_A, CK_B)
};
int retry_count = 0;

float GPS_prev_lat = 0;
float GPS_prev_lon = 0;


// MPU //
MPU9250_t myMPU;
double gyro_bias[3] = {-0.9297, 2.2277, -1.0220};
double mag_bias[3] = {11.4028, 12.5664, 11.3446};
double cal_gyro[3];
double cal_mag[3];
static uint32_t imu_prev_tick = 0;
static uint32_t sd_prev_tick = 0;


// FATFS & microSD //
char sd_temp[512];
int current_len = 0; // SDIO 용 데이터 길이 확인용
__attribute__((aligned(4))) FATFS   fs;   // FATFS structure
FIL     file; // File object structure
FRESULT res;  // File function return code
UINT    bytes_written; // The number of bytes actually written


// RTC //
RTC_DateTypeDef sDate;
RTC_TimeTypeDef sTime;
uint8_t current_Hours = 0;
uint8_t current_Minutes = 0;
uint8_t current_Seconds = 0;
int current_SubSeconds = 0;

// Display and States
VehicleState VS;
DisplayTransmit DISP;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

// SPI2 통신 기본 PFP //
void SD_Init(void);
void SD_WriteFile(char *data, int len);
void SD_SaveFile(void);

// UART 통신 기본 PFP //
int _write(int file, char *ptr, int len);                                                   // printf UART 출력
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
    int retry = 0;
    do {
        res = f_mount(&fs, "", 1);
        if (res == FR_OK) break;

        printf("SD Mount Fail (%d), retry %d\n", res, retry);
        f_mount(NULL, "", 0);   // 마운트 해제(정리)
        HAL_Delay(200);         // 카드가 안정될 시간
        retry++;
    } while (retry < 5);

	res = f_open(&file, "data.csv", FA_WRITE | FA_OPEN_APPEND);
	if (res == FR_OK) {
		printf("SD Card File Open Complete. \n");
	} else {
		printf("SD Card File Open Fail! Error Code : %d\n", res);
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
		printf("Sync Complete!\n");
	} else {
		printf("Sync Failed! Error Code : %d\n", res);
	}
}

// RTC //
void get_time(void){
	HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BCD);
	HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BCD);
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

  printf("CAN Setup and Test...\n");

  CAN_Config();

  printf("SDIO Setup...\n");

  SD_Init();

  printf("GPS Setup...\n");

  HAL_UART_Transmit(&huart3, ubx_rate_5hz, sizeof(ubx_rate_5hz), 100);

  GPS_Init();

  printf("MPU Starting...\n");

  DisplayTransmit_Init(&DISP, &VS);

  // 문제 없으면 지워버릴것. 아마도 점퍼선에 의한 문제인걸로 확인됨.
  retry_count = 0;
  while(MPU9250_Init(&myMPU, MPU9250_Device_0, ACCEL_SCALE_8G, GYRO_SCALE_2000dps, MAG_SCALE_16bit) != MPU9250_RESULT_OK)
  {
	  printf("MPU Setup Failed!... Retry count = %d\n", retry_count);

	  HAL_I2C_DeInit(&hi2c1);

	  HAL_Delay(100);

	  HAL_I2C_Init(&hi2c1);

	  retry_count ++;

	  if(retry_count >= 5) {
		  printf("MPU is Dead...\n");
		  Error_Handler();
	  }
  }
  printf("MPU Success!\n");

  get_time();
  printf("\r\n20%02x-%02x-%02x %02x:%02x", sDate.Year, sDate.Month, sDate.Date,
  												sTime.Hours, sTime.Minutes);


  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  HAL_IWDG_Refresh(&hiwdg);
	  get_time();
	  current_Hours = sTime.Hours;
	  current_Minutes = sTime.Minutes;
	  current_Seconds = sTime.Seconds;
	  float subsec = (float)(255 - sTime.SubSeconds) / 256.0f;
	  current_SubSeconds = (int)(subsec*1000);

	  if(IsPopAvailable())
	  {
	          RxMessage received = RingPop();

	          // 1. 변수 초기화
	          current_len = 0; // 현재까지 쓴 글자 수 (커서 위치)

	          // 2. 시간 쓰기
	          current_len += sprintf(sd_temp, "%02x:%02x:%02x.%03x", current_Hours, current_Minutes,
	        		  	  	  	  	  	  	  	  	  	  	  	  current_Seconds, current_SubSeconds);

	          // 3. CAN ID 쓰기
	          current_len += sprintf(sd_temp + current_len, ", %lx", received.RxHeader.StdId);

	          // 4. 데이터 반복문
	          for(int i = 0; i < received.RxHeader.DLC; i++) {
	              current_len += sprintf(sd_temp + current_len, ", %d", received.RxData[i]);
	          }

	          // 5. 문단 나누기 (1행 종료)
	          current_len += sprintf(sd_temp + current_len, "\n");

	          // 6. 화면 출력
	          printf("CAN %lx : %s", received.RxHeader.StdId, sd_temp);

	          // 7. SD 카드 저장
	          SD_WriteFile(sd_temp, current_len);
			  Classifier(&VS, received);
	      }

	  if(GPS.dec_latitude != GPS_prev_lat || GPS.dec_longitude != GPS_prev_lon)
	  {
		  // 1. 변수 초기화
		  current_len = 0;

		  // 2. 시간 쓰기
          current_len += sprintf(sd_temp, "%02x:%02x:%02x.%03x", current_Hours, current_Minutes,
        		  	  	  	  	  	  	  	  	  	  	  	  current_Seconds, current_SubSeconds);

		  // 3. GPS 인덱스 쓰기
		  current_len += sprintf(sd_temp + current_len, ", GPS");

		  // 4. 데이터 쓰기
		  current_len += sprintf(sd_temp + current_len, ", %f, %f",
				  GPS.dec_latitude,
				  GPS.dec_longitude);

		  // 5. 문단 나누기 (1행 종료)
		  current_len += sprintf(sd_temp + current_len, "\n");

		  // 6. 화면 출력
  	  	  printf("GPS : %s", sd_temp);

  	  	  // 7. SD 카드 저장
  	  	  SD_WriteFile(sd_temp, current_len);

  	  	  // 8. GPS 데이터 임시 저장 후 비교
  	  	  GPS_prev_lat = GPS.dec_latitude;
  	  	  GPS_prev_lon = GPS.dec_longitude;
	  }

	  if(HAL_GetTick() - imu_prev_tick >= 100) {
		  // 0. IMU 데이터 읽기
		  imu_prev_tick = HAL_GetTick();

	      MPU9250_ReadAcc(&myMPU);
	      MPU9250_ReadGyro(&myMPU);
	      MPU9250_ReadMag(&myMPU);

		  cal_gyro[0] = myMPU.gyro[0] - gyro_bias[0];
		  cal_gyro[1] = myMPU.gyro[1] - gyro_bias[1];
		  cal_gyro[2] = myMPU.gyro[2] - gyro_bias[2];

		  cal_mag[0] = myMPU.mag[0] - mag_bias[0];
		  cal_mag[1] = myMPU.mag[1] - mag_bias[1];
		  cal_mag[2] = myMPU.mag[2] - mag_bias[2];

		  // 1. 변수 초기화
		  current_len = 0;

		  // 2. 시간 쓰기
          current_len += sprintf(sd_temp, "%02x:%02x:%02x.%03x", current_Hours, current_Minutes,
        		  	  	  	  	  	  	  	  	  	  	  	  current_Seconds, current_SubSeconds);

		  // 3. IMU 인덱스 쓰기
		  current_len += sprintf(sd_temp + current_len, ", IMU");

		  // 4. 데이터 쓰기
		  current_len += sprintf(sd_temp + current_len,
				  ", %f, %f, %f, %f, %f, %f, %f, %f, %f",
				  myMPU.acc[0], myMPU.acc[1], myMPU.acc[2],
				  cal_gyro[0], cal_gyro[1], cal_gyro[2],
				  cal_mag[0], cal_mag[1], cal_mag[2]);

		  // 5. 문단 나누기 (1행 종료)
		  current_len += sprintf(sd_temp + current_len, "\n");

		  // 6. 화면 출력
		  //printf("IMU : %s", sd_temp);

		  // 7. SD 카드 저장
		  SD_WriteFile(sd_temp, current_len);
		  VehicleState_set_lat_g(&VS,  (uint16_t)(myMPU.acc[0] * 10.0f + 30.0f));
		  VehicleState_set_long_g(&VS, (uint16_t)(myMPU.acc[1] * 10.0f + 30.0f));
	  }

	  if (HAL_GetTick() - sd_prev_tick >= 1000)
	  {
		  SD_SaveFile();
		  sd_prev_tick = HAL_GetTick();

	  }

	  DisplayTransmit_Transmit(&DISP);
	  HAL_Delay(10);
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

// GPS Call Back (USART3) //
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if(huart == &huart3) GPS_UART_CallBack();
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
