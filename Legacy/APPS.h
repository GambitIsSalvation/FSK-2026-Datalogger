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

#define ADS1115_MAX_CODE  32767.0f
#define ADS1115_FSR       6.144f

#define APPS1_VALID_MIN  0.50f
#define APPS1_VALID_MAX  4.50f

#define APPS1_POS_MIN     0.50f
#define APPS1_POS_MAX     3.10f

#define APPS2_VALID_MIN  0.165f
#define APPS2_VALID_MAX  1.485f

#define APPS2_POS_MIN     0.165f
#define APPS2_POS_MAX     1.045f

#define DIFF_PCT		10.0f	// 10% 차이
#define NONVALID_TIME	100u    // 100ms
#define LOOP_PERIOD		5u      // 5ms 루프

float absf(float x);
float adc_to_voltage(int16_t adc);
float voltage_to_position(float voltage,float v_min,float v_max);
uint8_t apps_process(void);
void APPS_Init(void);
void APPS (void);
uint16_t get_p(void);

