 /*
 * gps.c
 *
 *  Created on: Nov 15, 2019
 *      Author: Bulanov Konstantin
 *
 *  Contact information
 *  -------------------
 *
 * e-mail   :  leech001@gmail.com
 */

/*
 * |---------------------------------------------------------------------------------
 * | Copyright (C) Bulanov Konstantin,2019
 * |
 * | This program is free software: you can redistribute it and/or modify
 * | it under the terms of the GNU General Public License as published by
 * | the Free Software Foundation, either version 3 of the License, or
 * | any later version.
 * |
 * | This program is distributed in the hope that it will be useful,
 * | but WITHOUT ANY WARRANTY; without even the implied warranty of
 * | MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * | GNU General Public License for more details.
 * |
 * | You should have received a copy of the GNU General Public License
 * | along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * |---------------------------------------------------------------------------------
 */


#include <stdio.h>
#include <string.h>
#include <usart.h>
#include "gps.h"
#include "main.h"

#if (GPS_DEBUG == 1)
#include <usbd_cdc_if.h>
#endif

uint8_t rx_data = 0;
uint8_t rx_buffer[GPSBUFSIZE];
uint8_t rx_index = 0;

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

uint8_t ubx_rate_10hz[] =    // Changing GPS Update Rate Protocol
{
    0xB5, 0x62,       		// Header
    0x06, 0x08,       		// Class(CFG), ID(RATE)
    0x06, 0x00,       		// Length (6 bytes)
    0x64, 0x00,       		// measRate (200ms = 5Hz) (Little Endian)
    0x01, 0x00,       		// navRate (1 cycle)
    0x01, 0x00,       		// timeRef (GPS Time)
    0x7A, 0x12        		// Checksum (CK_A, CK_B)
};

uint8_t ubx_baudrate_115200[] =
{
		0xB5, 0x62, 0x06, 0x00, 0x14, 0x00,
		0x01, 0x00, 0x00, 0x00, 0xD0, 0x08,
		0x00, 0x00, 0x00, 0xC2, 0x01, 0x00,
		0x07, 0x00, 0x03, 0x00, 0x00, 0x00,
		0x00, 0x00, 0xC0, 0x7E
};


GPS_t GPS;

#if (GPS_DEBUG == 1)
void GPS_print(char *data){
	char buf[GPSBUFSIZE] = {0,};
	sprintf(buf, "%s\n", data);
	CDC_Transmit_FS((unsigned char *) buf, (uint16_t) strlen(buf));
}
#endif

void GPS_Init()
{
	//HAL_UART_Transmit(&huart3, ubx_rate_5hz, sizeof(ubx_rate_5hz), 100);

	if(HAL_UART_Transmit(&huart3, ubx_baudrate_115200, sizeof(ubx_baudrate_115200), 100) == HAL_OK) {
		HAL_Delay(100);
		HAL_UART_DeInit(&huart3);
		huart3.Init.BaudRate = 115200;
		if(HAL_UART_Init(&huart3) != HAL_OK) {
			init_status |= (1 << 2);
		}
	}
	if(HAL_UART_Transmit(&huart3, ubx_rate_10hz, sizeof(ubx_rate_10hz), 100) != HAL_OK) {
		init_status |= (1 << 2);
	}
	if(HAL_UART_Receive_IT(GPS_USART, &rx_data, 1) != HAL_OK) { init_status |= (1 << 2); }
}


void GPS_UART_CallBack(){
	if (rx_data != '\n' && rx_index < sizeof(rx_buffer)) {
		rx_buffer[rx_index++] = rx_data;
	} else {

		#if (GPS_DEBUG == 1)
		GPS_print((char*)rx_buffer);
		#endif

		if(GPS_validate((char*) rx_buffer))
			GPS_parse((char*) rx_buffer);
		rx_index = 0;
		memset(rx_buffer, 0, sizeof(rx_buffer));
	}
	HAL_UART_Receive_IT(GPS_USART, &rx_data, 1);
}


int GPS_validate(char *nmeastr){
    char check[3];
    char checkcalcstr[3];
    int i;
    int calculated_check;

    i=0;
    calculated_check=0;

    // check to ensure that the string starts with a $
    if(nmeastr[i] == '$')
        i++;
    else
        return 0;

    //No NULL reached, 75 char largest possible NMEA message, no '*' reached
    while((nmeastr[i] != 0) && (nmeastr[i] != '*') && (i < 75)){
        calculated_check ^= nmeastr[i];// calculate the checksum
        i++;
    }

    if(i >= 75){
        return 0;// the string was too long so return an error
    }

    if (nmeastr[i] == '*'){
        check[0] = nmeastr[i+1];    //put hex chars in check string
        check[1] = nmeastr[i+2];
        check[2] = 0;
    }
    else
        return 0;// no checksum separator found there for invalid

    sprintf(checkcalcstr,"%02X",calculated_check);
    return((checkcalcstr[0] == check[0])
        && (checkcalcstr[1] == check[1])) ? 1 : 0 ;
}

void GPS_parse(char *GPSstrParse){
    // "$GPGGA" 든 "$GNGGA" 든 뒤 3글자(GGA)로 판별
    if(!strncmp(&GPSstrParse[3], "GGA", 3)){
        // $ + (GP/GN 2글자를 %*2c로 건너뜀) + GGA
        if (sscanf(GPSstrParse, "$%*2cGGA,%f,%f,%c,%f,%c,%d,%d,%f,%f,%c",
                   &GPS.utc_time, &GPS.nmea_latitude, &GPS.ns,
                   &GPS.nmea_longitude, &GPS.ew, &GPS.lock,
                   &GPS.satelites, &GPS.hdop, &GPS.msl_altitude,
                   &GPS.msl_units) >= 1){
            GPS.dec_latitude  = GPS_nmea_to_dec(GPS.nmea_latitude, GPS.ns);
            GPS.dec_longitude = GPS_nmea_to_dec(GPS.nmea_longitude, GPS.ew);
            return;
        }
    }
    else if (!strncmp(&GPSstrParse[3], "RMC", 3)){
        if(sscanf(GPSstrParse, "$%*2cRMC,%f,%f,%c,%f,%c,%f,%f,%d",
                  &GPS.utc_time, &GPS.nmea_latitude, &GPS.ns,
                  &GPS.nmea_longitude, &GPS.ew, &GPS.speed_k,
                  &GPS.course_d, &GPS.date) >= 1)
            return;
    }
    else if (!strncmp(&GPSstrParse[3], "GLL", 3)){
        if(sscanf(GPSstrParse, "$%*2cGLL,%f,%c,%f,%c,%f,%c",
                  &GPS.nmea_latitude, &GPS.ns, &GPS.nmea_longitude,
                  &GPS.ew, &GPS.utc_time, &GPS.gll_status) >= 1)
            return;
    }
    else if (!strncmp(&GPSstrParse[3], "VTG", 3)){
        if(sscanf(GPSstrParse, "$%*2cVTG,%f,%c,%f,%c,%f,%c,%f,%c",
                  &GPS.course_t, &GPS.course_t_unit, &GPS.course_m,
                  &GPS.course_m_unit, &GPS.speed_k, &GPS.speed_k_unit,
                  &GPS.speed_km, &GPS.speed_km_unit) >= 1)
            return;
    }
}

float GPS_nmea_to_dec(float deg_coord, char nsew) {
    int degree = (int)(deg_coord/100);
    float minutes = deg_coord - degree*100;
    float dec_deg = minutes / 60;
    float decimal = degree + dec_deg;
    if (nsew == 'S' || nsew == 'W') { // return negative
        decimal *= -1;
    }
    return decimal;
}

void print_rxdata(void) {
	for (int i = 0; i < GPSBUFSIZE; i++){
		printf("%c", rx_buffer[i]);
	}
	printf("\n");
}
