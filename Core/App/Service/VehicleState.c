/*
 * VehicleState.c
 */

#include "VehicleState.h"

static const TempPoint temp_igbt[] = {
    {16245, -35}, {16308, -30}, {16387, -25}, {16487, -20},
    {16609, -15}, {16759, -10}, {16938, -5},  {17151, 0},
    {17400, 5},   {17688, 10},  {18017, 15},  {18387, 20},
    {18797, 25},  {19247, 30},  {19733, 35},  {20250, 40},
    {20793, 45},  {21357, 50},  {21933, 55},  {22515, 60},
    {23097, 65},  {23671, 70},  {24232, 75},  {24775, 80},
    {25296, 85},  {25792, 90},  {26261, 95},  {26702, 100},
    {27114, 105}, {27497, 110}, {27851, 115}, {28179, 120},
    {28480, 125}, {28757, 130}, {29011, 135}, {29243, 140},
    {29456, 145}, {29650, 150}, {29827, 155}
};


void VehicleState_Init(VehicleState* self)
{
    self->rpm = 0;
    self->apps = 0;
    self->soc = 0;
    self->mtr_temp = 0;
    self->ctrl_temp = 0;
    self->htp = 0;
    self->hcv = 0;
    self->lcv = 0;
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
void VehicleState_set_lcv(VehicleState* self, uint16_t val)       { self->lcv = val; }
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
uint16_t VehicleState_get_lcv(const VehicleState* self)		  { return self->lcv; }
uint16_t VehicleState_get_lat_g(const VehicleState* self)     { return self->lat_g; }
uint16_t VehicleState_get_long_g(const VehicleState* self)    { return self->long_g; }
uint16_t VehicleState_get_laps(const VehicleState* self)      { return self->laps; }
uint32_t VehicleState_get_lap_time(const VehicleState* self)  { return self->lap_time; }


// motor temp process
int16_t igbt_raw_to_temp_x10(uint16_t raw) {
    if (raw <= temp_igbt[0].raw)
        return temp_igbt[0].temp * 10;
    if (raw >= temp_igbt[38].raw)
        return temp_igbt[38].temp * 10;

    for (int i = 0; i < 38; i++) {
        if (raw <= temp_igbt[i+1].raw) {
            int32_t temp_diff = (temp_igbt[i+1].temp - temp_igbt[i].temp) * 10;
            int32_t raw_span  = temp_igbt[i+1].raw - temp_igbt[i].raw;
            int32_t raw_offset = raw - temp_igbt[i].raw;
            int32_t result = temp_igbt[i].temp * 10
                           + (raw_offset * temp_diff) / raw_span;
            return (int16_t)result;
        }
    }
    return temp_igbt[38].temp * 10;
}
