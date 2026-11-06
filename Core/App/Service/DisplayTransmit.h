/*
 * DisplayTransmit.h
 *
 *  Created on: 2026. 5. 6.
 *      Author: leesa
 */
#ifndef DISPLAYTRANSMIT_H
#define DISPLAYTRANSMIT_H

#include "VehicleState.h"
#include <stdint.h>

#define DISP_TX_BUFFER_SIZE 26

typedef struct {
    VehicleState* VS;
    uint8_t tx_buffer[DISP_TX_BUFFER_SIZE];
} DisplayTransmit;

void DisplayTransmit_Init(DisplayTransmit* self, VehicleState* vs);
void DisplayTransmit_Packing(DisplayTransmit* self);
void DisplayTransmit_Transmit(DisplayTransmit* self);

#endif /* APP_DISPLAYTRANSMIT_H_ */
