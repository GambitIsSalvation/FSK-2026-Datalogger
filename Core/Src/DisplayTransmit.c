#include "DisplayTransmit.h"
#include "VehicleState.h"
#include <string.h>
#include "main.h"

extern UART_HandleTypeDef huart6;
volatile uint8_t disp_dma_ready = 1;

void DisplayTransmit_Init(DisplayTransmit* self, VehicleState* vs)
{
    self->VS = vs;
    memset(self->tx_buffer, 0, sizeof(self->tx_buffer));
}

void DisplayTransmit_Packing(DisplayTransmit* self)
{
    // Big Endian ( 0xABCD -> 0xAB, 0xCD 순서 )
    uint8_t* buf = self->tx_buffer;
    uint16_t temp;

    memset(buf, 0, DISP_TX_BUFFER_SIZE);

    // 헤더
    buf[0] = 0xAA;
    buf[1] = 0xBB;

    // 데이터 패킹
    temp = VehicleState_get_rpm(self->VS);
    buf[2] = (uint8_t)(temp >> 8);
    buf[3] = (uint8_t)(temp);
/*
    temp = VehicleState_get_apps(self->VS);
    buf[4] = (uint8_t)(temp >> 8);
    buf[5] = (uint8_t)(temp);
*/
    temp = VehicleState_get_soc(self->VS);
    buf[4] = (uint8_t)(temp >> 8);
    buf[5] = (uint8_t)(temp);

    temp = VehicleState_get_mtr_temp(self->VS);
    buf[6] = (uint8_t)(temp >> 8);
    buf[7] = (uint8_t)(temp);

    temp = VehicleState_get_ctrl_temp(self->VS);
    buf[8] = (uint8_t)(temp >> 8);
    buf[9] = (uint8_t)(temp);

    temp = VehicleState_get_htp(self->VS);
    buf[10] = (uint8_t)(temp >> 8);
    buf[11] = (uint8_t)(temp);

    temp = VehicleState_get_hcv(self->VS);
    buf[12] = (uint8_t)(temp >> 8);
    buf[13] = (uint8_t)(temp);

    temp = VehicleState_get_lcv(self->VS);
    buf[14] = (uint8_t)(temp >> 8);
    buf[15] = (uint8_t)(temp);

    temp = VehicleState_get_lat_g(self->VS);
    buf[16] = (uint8_t)(temp >> 8);
    buf[17] = (uint8_t)(temp);

    temp = VehicleState_get_long_g(self->VS);
    buf[18] = (uint8_t)(temp >> 8);
    buf[19] = (uint8_t)(temp);

    /*
    temp = VehicleState_get_laps(self->VS);
    buf[20] = (uint8_t)(temp >> 8);
    buf[21] = (uint8_t)(temp);

    temp = VehicleState_get_lap_time(self->VS);
    buf[22] = (uint8_t)(temp >> 24);
    buf[23] = (uint8_t)(temp >> 16);
    buf[24] = (uint8_t)(temp >> 8);
    buf[25] = (uint8_t)(temp);
*/
    // buf[26], buf[27] 체크섬 추가 예정
}

void ErrorPacking(DisplayTransmit* self){
    // Big Endian ( 0xABCD -> 0xAB, 0xCD 순서 )
    uint8_t* buf = self->tx_buffer;
    uint32_t temp;

    memset(buf, 0, DISP_TX_BUFFER_SIZE);

    // 헤더
    buf[0] = 0xEE;
    buf[1] = 0xFF;

    temp = self->VS->bms_error;
    buf[2] = (uint8_t)(temp >> 24);
    buf[3] = (uint8_t)(temp >> 16);
    buf[4] = (uint8_t)(temp >> 8);
    buf[5] = (uint8_t)(temp);
}

void DisplayTransmit_Transmit(DisplayTransmit* self)
{
	if (disp_dma_ready) {
		disp_dma_ready = 0;
		DisplayTransmit_Packing(self);
		HAL_UART_Transmit_DMA(&huart6, self->tx_buffer, sizeof(self->tx_buffer));
		if(self->VS->bms_error != 0){
			uint32_t timeout = HAL_GetTick();
			while(huart6.gState != HAL_UART_STATE_READY){
				if(HAL_GetTick() - timeout > 5) break;
			}
			ErrorPacking(self);
			HAL_UART_Transmit_DMA(&huart6, self->tx_buffer, sizeof(self->tx_buffer));
		}
	}
}
