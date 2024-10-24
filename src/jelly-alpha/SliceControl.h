#ifndef SLICE_CONTROL_H
#define SLICE_CONTROL_H

#include "DataStructures.h"

// Function declarations for initializing slices and I2C control
void initSlices();
void RLHTRequestThermo(int address, float *t1, float *t2);
void RLHTCommandSetpoint(int address, byte heater, float heatSetpoint, byte thermocouple, bool enableReverse);
void RLHTCommandPID(int address, byte heater, float Kp_set, float Ki_set, float Kd_set);
void DCMTCommandSpeed(int address, byte motor, int speed_set, bool enable);
void DCMTCommandPH(int address, float pHSetpoint_set, float currentPH_set);
void DCMTCommandPHPID(int address, float Kp_set, float Ki_set, float Kd_set);
void DCMTCommandTurbidity(int address, byte motor, byte direction, bool enable, int sample_period);
void DCMTRequestTurbidity(int address, float *turbidity1, float *turbidity2);
void PHDOCommand(int address, const char *commandData);
float PHDORequest(int address);

#endif
