/*
 * Laptime.c
 *
 *  Created on: 2026. 6. 26.
 *      Author: leesa
 */


#include "Laptime.h"
#include <stdint.h>

uint16_t time_norm = 0;
uint16_t laptime = 0;
uint16_t lap = 0;

void lap_setup(void)
{
	time_norm = HAL_GetTick();
}

void lap_end(void)
{
	laptime = HAL_GetTick() - time_norm;
	lap++;
	time_norm = HAL_GetTick();
}

uint16_t get_lap(void)
{
	return lap;
}

uint16_t get_laptime(void)
{
	return laptime;
}
