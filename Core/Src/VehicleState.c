/*
 * VehicleState.c
 */

#include "VehicleState.h"

void VehicleState_Init(VehicleState* self)
{
    self->rpm = 0;
    self->apps = 0;
    self->soc = 0;
    self->mtr_temp = 0;
    self->ctrl_temp = 0;
    self->htp = 0;
    self->hcv = 0;
    self->lat_g = 30;   // 0G 기준 오프셋
    self->long_g = 30;  // 0G 기준 오프셋
    self->laps = 1;
    self->lap_time = 0;
}

// Setters
void VehicleState_set_rpm(VehicleState* self, uint16_t val) { self->rpm = val; }

void VehicleState_set_apps(VehicleState* self, uint16_t val)
{
    if (val > 1000) val = 1000;
    self->apps = val;
}

void VehicleState_set_soc(VehicleState* self, uint16_t val)
{
    if (val > 100) val = 100;
    self->soc = val;
}

void VehicleState_set_mtr_temp(VehicleState* self, uint16_t val)  { self->mtr_temp = val; }
void VehicleState_set_ctrl_temp(VehicleState* self, uint16_t val) { self->ctrl_temp = val; }
void VehicleState_set_htp(VehicleState* self, uint16_t val)       { self->htp = val; }
void VehicleState_set_hcv(VehicleState* self, uint16_t val)       { self->hcv = val; }
void VehicleState_set_lat_g(VehicleState* self, uint16_t val)     { self->lat_g = val; }
void VehicleState_set_long_g(VehicleState* self, uint16_t val)    { self->long_g = val; }
void VehicleState_set_laps(VehicleState* self, uint16_t val)      { self->laps = val; }
void VehicleState_set_lap_time(VehicleState* self, uint16_t val)  { self->lap_time = val; }

// Getters
uint16_t VehicleState_get_rpm(const VehicleState* self)       { return self->rpm; }
uint16_t VehicleState_get_apps(const VehicleState* self)      { return self->apps; }
uint16_t VehicleState_get_soc(const VehicleState* self)       { return self->soc; }
uint16_t VehicleState_get_mtr_temp(const VehicleState* self)  { return self->mtr_temp; }
uint16_t VehicleState_get_ctrl_temp(const VehicleState* self) { return self->ctrl_temp; }
uint16_t VehicleState_get_htp(const VehicleState* self)       { return self->htp; }
uint16_t VehicleState_get_hcv(const VehicleState* self)       { return self->hcv; }
uint16_t VehicleState_get_lat_g(const VehicleState* self)     { return self->lat_g; }
uint16_t VehicleState_get_long_g(const VehicleState* self)    { return self->long_g; }
uint16_t VehicleState_get_laps(const VehicleState* self)      { return self->laps; }
uint16_t VehicleState_get_lap_time(const VehicleState* self)  { return self->lap_time; }
