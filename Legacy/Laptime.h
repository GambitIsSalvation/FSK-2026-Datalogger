/*
 * Laptime.h
 *
 *  Created on: 2026. 6. 26.
 *      Author: leesa
 */
#include <stdint.h>

#ifndef INC_LAPTIME_H_
#define INC_LAPTIME_H_

void lap_setup(void);
void lap_end(void);
uint16_t get_lap(void);
uint16_t get_laptime(void);

#endif /* INC_LAPTIME_H_ */
