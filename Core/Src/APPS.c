/*
 * APPS.c
 *
 *  Created on: 2026. 6. 26.
 *      Author: leesa
 */

#include "main.h"
#include "APPS.h"
#include "i2c.h"
#include "ADS1015_ADS1115.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

ADS1xx5_I2C i2c;
float pos1;
float pos2;

// 절댓값
float absf(float x)
{
    return (x < 0.0f) ? -x : x;
}


// ADC 값 → 전압
float adc_to_voltage(int16_t adc)
{
    return ((float)adc / ADS1115_MAX_CODE) * ADS1115_FSR;
}


// 전압 → 페달 위치량(%)
float voltage_to_position(float voltage,
                          float v_min,
                          float v_max)
{
    if (voltage <= v_min)
        return 0.0f;

    if (voltage >= v_max)
        return 100.0f;

    return ((voltage - v_min) /
            (v_max - v_min)) * 100.0f;
}


// 정상 → 1
// 100 ms 이상 비정상 → 0
uint8_t apps_process(void)
{
    int16_t adc1;
    int16_t adc2;

    float v1;
    float v2;

    pos1;
    pos2;

    float diff;

    bool apps1_valid;
    bool apps2_valid;

    static bool implausible_active = false;
    static uint32_t implausible_start = 0;

    uint32_t now;


    adc1 = ADSreadADC_SingleEnded(&i2c, 0);
    adc2 = ADSreadADC_SingleEnded(&i2c, 1);


    v1 = adc_to_voltage(adc1);
    v2 = adc_to_voltage(adc2);


    apps1_valid = (v1 >= APPS1_VALID_MIN &&
                   v1 <= APPS1_VALID_MAX);

    apps2_valid = (v2 >= APPS2_VALID_MIN &&
                   v2 <= APPS2_VALID_MAX);


    pos1 = voltage_to_position(v1,
                               APPS1_POS_MIN,
                               APPS1_POS_MAX);

    pos2 = voltage_to_position(v2,
                               APPS2_POS_MIN,
                               APPS2_POS_MAX);


    diff = absf(pos1 - pos2);



    if (apps1_valid &&
        apps2_valid &&
        diff <= DIFF_PCT)
    {
        // 정상으로 돌아오면 타이머 초기화
        implausible_active = false;
        implausible_start = 0;

        // 정상 → 0
        return 0;
    }



    now = HAL_GetTick();

    if (!implausible_active)
    {
        implausible_active = true;
        implausible_start = now;

        // 아직 100 ms가 지나지 않음
        return 0;
    }

    if ((now - implausible_start) >= NONVALID_TIME)
    {
        // 100 ms 이상 지속 → FAULT
        return 1;
    }

    // 100 ms 미만 → 아직 정상 출력 유지
    return 0;
}

void APPS_Init(void) {
	  ADS1115(&i2c, &hi2c2, ADS_ADDR_GND);
	  ADSsetGain(&i2c, GAIN_TWOTHIRDS);

	  // 초기 MOTOR OFF
	  HAL_GPIO_WritePin(APPS_OUT_GPIO_Port,
			  	  	  	  APPS_OUT_Pin,
	                    GPIO_PIN_SET);
}

void APPS(void) {
	  HAL_GPIO_WritePin(APPS_OUT_GPIO_Port,
  	  	  	  	  	  	  APPS_OUT_Pin,
	                        apps_process() ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

uint16_t get_p(void) {
	return pos1;
}

