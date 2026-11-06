/*
 * CrossDetect.h
 *
 *  Created on: 2026. 7. 13.
 *      Author: leesa
 */
#include "main.h"

#ifndef INC_CROSSDETECT_H_
#define INC_CROSSDETECT_H_

#define START_LINE_LAT 37.55074
#define START_LINE_LON 126.92535
#define FINISH_LINE_LAT 37.55090
#define FINISH_LINE_LON 126.92535
#define MIN_LAP_MS 5000u


uint8_t get_line_intersection(float p0_lat, float p0_lon,
                          float p1_lat, float p1_lon);

#endif /* INC_CROSSDETECT_H_ */
