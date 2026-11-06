#include <stdio.h>
#include <math.h>
#include "CrossDetect.h"

uint8_t laps = 1;
uint32_t lap_time = 0;
uint32_t pre_tick = 0;

uint8_t get_line_intersection(float p0_lat, float p0_lon,
                          float p1_lat, float p1_lon)
{
	if (pre_tick != 0 && (HAL_GetTick() - pre_tick) < MIN_LAP_MS) {
	    return 0;
	}

    // 이동 벡터 r, s
    float r_lat = p1_lat - p0_lat;
    float r_lon = p1_lon - p0_lon;

    float s_lat = FINISH_LINE_LAT - START_LINE_LAT;
    float s_lon = FINISH_LINE_LON - START_LINE_LON;

    // r × s (2D 외적)
    float r_cross_s = r_lat * s_lon - r_lon * s_lat;

    // 평행 또는 동일선상 → 교차 없음
    if (fabsf(r_cross_s) < 1e-12f) {
        return 0;
    }

    float q_minus_p_lat = START_LINE_LAT - p0_lat;
    float q_minus_p_lon = START_LINE_LON - p0_lon;

    float inv_r_cross_s = 1.0f / r_cross_s;

    float t = (q_minus_p_lat * s_lon - q_minus_p_lon * s_lat) * inv_r_cross_s;
    float u = (q_minus_p_lat * r_lon - q_minus_p_lon * r_lat) * inv_r_cross_s;

    if (t >= 0.0f && t <= 1.0f && u >= 0.0f && u <= 1.0f)
    {
        laps++;
        lap_time = HAL_GetTick() - pre_tick;
        pre_tick = HAL_GetTick();
        return 1;
    }

    return 0;
}
