/*
 * APPS.c
 *
 *  Created on: 2026. 6. 26.
 *      Author: leesa
 */
#include "APPS.h"
#include "main.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

uint16_t potVal[2];
char data[160];

static apps_state_t apps_state = APPS_OK;	// 현재 APPS 상태값
static uint32_t implausible_start = 0;		// 비타당성 누적 시간
static uint32_t last_print = 0;
static bool fault_latched = false;			// 최종값

float p1 = 0;
float p2 = 0;

static float absf(float x){ return (x < 0.0f) ? -x : x; }

// adc값 V로 환산
static float adc_to_v(uint16_t adc)
{
  return ((float)adc / ADC_MAX) * ADC_VREF;
}

// 동작범위 확인 (유효값인지)
static bool in_range(float v, float vmin, float vmax)
{
  return (v >= vmin) && (v <= vmax);
}

// 전압값을 위치값 0~100% 로 환산 (기울기 다름)
static float v_to_pct(float v, float vmin, float vmax)
{
  if (v <= vmin) return 0.0f;
  if (v >= vmax) return 100.0f;
  return ( (v - vmin) / (vmax - vmin) ) * 100.0f;
}

// 결과 출력 (fault이면 MOTOR_SIG Pin에 High + LED ON)
static void show_output(bool fault)
{
	if (fault) {
		HAL_GPIO_WritePin(MOTOR_SIG_GPIO_Port, MOTOR_SIG_Pin, GPIO_PIN_SET);
	} else {
	    HAL_GPIO_WritePin(MOTOR_SIG_GPIO_Port, MOTOR_SIG_Pin, GPIO_PIN_RESET);
	}
}

void APPS (void)
{
	 uint32_t now = HAL_GetTick();

		  // 1ms 주기 (1ms 마다 값 측정 및 판단)
		  static uint32_t last_tick = 0;
		  if ((now - last_tick) < LOOP_PERIOD) {
		      return;
		  }
		  last_tick = now;

	    /* USER CODE END WHILE */

	    /* USER CODE BEGIN 3 */
		  // 전압 변환
		  float v1 = adc_to_v(potVal[0]);
		  float v2 = adc_to_v(potVal[1]);

		  // 동작 범위 확인
		  bool v1_in_range = in_range(v1, APPS1_VALID_MIN, APPS1_VALID_MAX);
		  bool v2_in_range = in_range(v2, APPS2_VALID_MIN, APPS2_VALID_MAX);

		  // 페달 위치 %로 환산
		  p1 = v_to_pct(v1, APPS1_VALID_MIN, APPS1_VALID_MAX);
		  p2 = v_to_pct(v2, APPS2_VALID_MIN, APPS2_VALID_MAX);

		  // 상태 판정
		  apps_state_t current_state = APPS_OK;

		  if (!v1_in_range || !v2_in_range) {		// 유효값인지
			  current_state = APPS_FAIL_FAILURE;
		  } else if (absf (p1 - p2) >= DIFF_PCT) {	// 오차값 10% 이내인지
			  current_state = APPS_FAIL_IMPLAUSIBLE;
		  } else {
			  current_state = APPS_OK;
		  }

		  // 만약 fault 발생했을 경우 누적 시간 측정
		  if (current_state != APPS_OK) {
			  if (apps_state == APPS_OK) {
				  implausible_start = now;	// fault 시작 시간 저장
				  fault_latched = false;

			  } else {
				  // 100ms 이상 지속되었을 때 최종 fault 판정
				  if (!fault_latched && (now - implausible_start >= NONVALID_TIME)) {
					  fault_latched = true;
				  }
			  }
		  } else {
			  fault_latched = false;
			  implausible_start = 0;
		  }

		  // 현재 상태 저장
		  apps_state = current_state;

		  // 결과 LED 출력
		  show_output(fault_latched);
}

uint16_t get_p(void)
{
	return p1;
}
