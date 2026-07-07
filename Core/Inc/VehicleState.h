/*
 * VehicleState.h
 */

#ifndef APP_VEHICLESTATE_H_
#define APP_VEHICLESTATE_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define NMAX 6000 // rpm maximum value



// Field Index   Name        Type     unit / range
// 0             rpm         uint16   rev/min
// 1             apps        uint16   0.0 - 100 % (x10 스케일, 0~1000)
// 2             soc         uint16   0 - 100 %
// 3             mtr_temp    uint16   'C (0.1도 정밀도)
// 4             ctrl_temp   uint16   'C (0.1도 정밀도)
// 5             htp         uint16   'C (BMS High Temp pack)
// 6             hcv         uint16   V (BMS High cell Voltage, 0.01V)
// 7             lat_g       uint16   G (Lateral Accel, 0~60, +30 오프셋)
// 8             long_g      uint16   G (Longitudinal Accel, 0~60, +30 오프셋)
// 9             laps        uint16
// 10            lap_time    uint16

typedef struct {
	/************* 디스플레이 + 데이터 로깅용 ***************/
	//BMS
    uint16_t soc;
    uint16_t htp;
    uint16_t hcv;
	//Motor
    int16_t rpm;
    uint16_t mtr_temp;
    uint16_t ctrl_temp;
	//Sensor
    uint16_t apps;
    uint16_t lat_g;
    uint16_t long_g;
    uint16_t laps;
    uint16_t lap_time;
    /************* 데이터 로깅용 ***************/
    //BMS
    uint16_t lcv; // Lowest Cell Voltage
    uint16_t ltp; // Lowest Temperature
    uint16_t current; // Pack Current
    uint16_t voltage; // Pack Voltage
    uint32_t bms_error; // Error Fault

    //Motor
    int16_t rpm_command; // target rpm
    int16_t iq_command;  // target effective current
    int16_t iq_actual;   // actual effective current
    uint16_t inv_vout;    // inverter to motor voltage
    uint16_t inv_vin;     // battery to inverter voltage


} VehicleState;

void VehicleState_Init(VehicleState* self);

// Setters
void VehicleState_set_rpm(VehicleState* self, uint16_t val);
void VehicleState_set_apps(VehicleState* self, uint16_t val);        // 0~1000 클램핑
void VehicleState_set_soc(VehicleState* self, uint16_t val);         // 0~100 클램핑
void VehicleState_set_mtr_temp(VehicleState* self, uint16_t val);
void VehicleState_set_ctrl_temp(VehicleState* self, uint16_t val);
void VehicleState_set_htp(VehicleState* self, uint16_t val);
void VehicleState_set_hcv(VehicleState* self, uint16_t val);
void VehicleState_set_lat_g(VehicleState* self, uint16_t val);
void VehicleState_set_long_g(VehicleState* self, uint16_t val);
void VehicleState_set_laps(VehicleState* self, uint16_t val);
void VehicleState_set_lap_time(VehicleState* self, uint16_t val);

// Getters
uint16_t VehicleState_get_rpm(const VehicleState* self);
uint16_t VehicleState_get_apps(const VehicleState* self);
uint16_t VehicleState_get_soc(const VehicleState* self);
uint16_t VehicleState_get_mtr_temp(const VehicleState* self);
uint16_t VehicleState_get_ctrl_temp(const VehicleState* self);
uint16_t VehicleState_get_htp(const VehicleState* self);
uint16_t VehicleState_get_hcv(const VehicleState* self);
uint16_t VehicleState_get_lat_g(const VehicleState* self);
uint16_t VehicleState_get_long_g(const VehicleState* self);
uint16_t VehicleState_get_laps(const VehicleState* self);
uint16_t VehicleState_get_lap_time(const VehicleState* self);

#ifdef __cplusplus
}
#endif

#endif /* APP_VEHICLESTATE_H_ */
