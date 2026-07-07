#include "ringbuffer.h"
#include "can.h"
#include "VehicleState.h"

// Ring Buffer Variabels //
RxMessage ringbuffer[RINGBUFFER_SIZE]; // 링 버퍼 정의
volatile uint8_t r_front = 0;
volatile uint8_t r_end = 0;
int16_t raw = 0;


// Ring Buffer Functions //

int IsWriteAvailable(void) // 링 버퍼가 가득 차있는지 확인
{
    if((r_front + 1) % RINGBUFFER_SIZE != r_end){
    	//printf("Write Available\n");
        return 1;
    } else {
    	//printf("Ring buffer is full!\n");
        return 0;
    }
}

int IsPopAvailable(void) // 링 버퍼에서 데이터를 추출할수 있는지 확인
{
    if(r_end != r_front){
    	//printf("Pop Available\n");
        return 1;
    } else {
    	//printf("Ring buffer is empty!\n");
        return 0;
    }
}

void RingFrontIncrement(void) // 링 버퍼 front 증가
{
    r_front = (r_front + 1) % RINGBUFFER_SIZE;
    return;
}

void RingEndIncrement(void) // 링 버퍼 end 증가
{
    r_end = (r_end + 1) % RINGBUFFER_SIZE;
    return;
}

RxMessage RingPop(void) // 링 버퍼에서 데이터 추출
{
	RxMessage poptemp = ringbuffer[r_end];
	RingEndIncrement();
	return poptemp;
}
void RingPush(RxMessage input) // 링 버퍼에 데이터 입력
{
	ringbuffer[r_front] = input;
	RingFrontIncrement();
	return;
}

void Classifier(VehicleState* VS, RxMessage received)
{
	switch(received.RxHeader.StdId)
	{
	// Motor Controller Loging Data
	case MOTORX:
		switch(received.RxData[0])
		{
		case RPM_COMMAND:
			raw = (int16_t)((received.RxData[2] << 8) | received.RxData[1]);
			VS->rpm_command = ((int32_t)raw * NMAX) / 32767;
			//VS->rpm_command = (int16_t)((received.RxData[2] << 8) | received.RxData[1]);
			break;

		case RPM_ACTUAL:
			raw = (int16_t)((received.RxData[2] << 8) | received.RxData[1]);
			VS->rpm = ((int32_t)raw * NMAX) / 32767;
			//VS->rpm = (int16_t)((received.RxData[2] << 8) | received.RxData[1]);
			break;

		case IQ_COMMAND:
			VS->iq_command = (int16_t)((received.RxData[2] << 8) | received.RxData[1]);
			break;

		case IQ_ACTUAL:
			VS->iq_actual = (int16_t)((received.RxData[2] << 8) | received.RxData[1]);
			break;

		case V_OUT:
			VS->inv_vout = (received.RxData[2] << 8) | received.RxData[1];
			break;

		case V_IN:
			VS->inv_vin = (received.RxData[2] << 8) | received.RxData[1];
			break;

		case T_CTRL:
			VS->ctrl_temp = (received.RxData[2] << 8) | received.RxData[1];
			break;

		case T_MOTOR:
			VS->mtr_temp = (received.RxData[2] << 8) | received.RxData[1];
			break;
		}
		break;

	// BMS Loging Data
	case BMSRX_1:
		VS->hcv = received.RxData[0];
		VS->lcv = received.RxData[1];
		VS->htp = received.RxData[2];
		VS->ltp = received.RxData[3];
		VS->soc = received.RxData[4];
		VS->current = received.RxData[5];
		VS->voltage = received.RxData[6];
		break;

	case BMSRX_2:
		VS->bms_error = ((uint32_t)received.RxData[0])
						| ((uint32_t)received.RxData[1] << 8)
						| ((uint32_t)received.RxData[2] << 16)
						| ((uint32_t)(received.RxData[3] & 0x07) << 24);
		break;
	}
}

