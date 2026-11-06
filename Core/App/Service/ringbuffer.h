#include "can.h"
#include "VehicleState.h"

// private defines //
#define RINGBUFFER_SIZE 1024 /// 링 버퍼 최대 크기 지정

// private function prototypes//
int IsWriteAvailable(void);        // 링 버퍼가 가득 차있는지 확인
int IsPopAvailable(void);          // 링 버퍼에서 데이터를 추출할수 있는지 확인
void RingFrontIncrement(void);                   // 링 버퍼 front 증가
void RingEndIncrement(void);                     // 링 버퍼 end 증가
RxMessage RingPop(void);                         // 링 버퍼에서 데이터 추출
void RingPush(RxMessage input);                  // 링 버퍼에 데이터 입력
void Classifier(VehicleState* VS, RxMessage received); //  CAN 메세지 분류
