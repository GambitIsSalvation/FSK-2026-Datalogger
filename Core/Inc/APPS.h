/*
 * APPS.
 *
 *  Created on: 2026. 6. 26.
 *      Author: leesa
 */

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "main.h"

#ifndef INC_APPS_C_
#define INC_APPS_C_

#define ADC_MAX			4095.0f
#define ADC_VREF		3.3f	// 전압 기준값 (3.3V)

#define APPS1_VALID_MIN	0.33f	// 위치 0%가 되는 전압 (0.33V)
#define APPS1_VALID_MAX	2.97f	// 위치 100%가 되는 전압 (2.97V)

#define APPS2_VALID_MIN	0.25f	// 위치 0%가 되는 전압 (0.25V)
#define APPS2_VALID_MAX	2.25f	// 위치 100%가 되는 전압 (2.25V)

#define DIFF_PCT		10.0f	// 10% 차이
#define NONVALID_TIME	100u    // 100ms
#define LOOP_PERIOD		1u      // 1ms 루프

#define MOTOR_SIG_GPIO_Port GPIOA
#define MOTOR_SIG_Pin GPIO_PIN_4

typedef enum {
	APPS_OK = 0,			// 정상
	APPS_FAIL_FAILURE,		// 유효값이 아닌 전압
	APPS_FAIL_IMPLAUSIBLE	// 비타당성
} apps_state_t;

void APPS (void);
uint16_t get_p(void);

#endif /* INC_APPS_C_ */
