#include "ringbuffer.h"
#include "can.h"

// Ring Buffer Variabels //
RxMessage ringbuffer[RINGBUFFER_SIZE]; // 링 버퍼 정의
volatile uint8_t r_front = 0;
volatile uint8_t r_end = 0;


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
