#ifndef DATA_STRUCTURES_H
#define DATA_STRUCTURES_H

#include <stdint.h>
#include <Arduino.h>

// Useful unions for breaking data into bytes
union FLOATUNION_t
{
    float number;
    uint8_t bytes[4];
};
union INTUNION_t
{
    int number;
    uint8_t bytes[4];
};

// Slice structures for managing data
struct DCMT_t
{
    int address; // I2C address of Slice
    int m1Speed; // motor 1 speed (-100-100)
    int m2Speed; // motor 2 speed (-100-100)
    bool m1En;   // motor 1 enable (0:off/apply brake, 1:on/coast)
    bool m2En;   // motor 2 enable (0:off/apply brake, 1:on/coast)
    float pHSetpoint;
    float currentPH;
    float Kp_pH;
    float Ki_pH;
    float Kd_pH;
};

struct RLHT_t
{
    int address;          // I2C address of Slice
    float heatSetpoint_1; // set desired temperature for heater 1 (if using relay as heater actuator)
    float Kp_1;           // PID proportional gain for heater 1
    float Ki_1;           // PID integral gain for heater 1
    float Kd_1;           // PID derivative gain for heater 1
    float heatSetpoint_2;
    float Kp_2;
    float Ki_2;
    float Kd_2;
    float thermo1; // thermocouple 1 measurement
    float thermo2; // thermocouple 2 measurement
};

struct PHDO_t
{
    int addressPH; // I2C address of Atlas Scientific pH EZO board (default = 99)
    int addressDO; // I2C address of Atlas Scientific dissolved oxygen EZO board (default = 97)
    float pH;      // current pH reading
    float DO;      // current dissolved oxygen reading
    bool calPH = false;
    bool calDO = false;
};

int pyro_thermo[9][2] = {
    // pyrolyis thermocouple layout {address, thermocouple}
    {16, 2}, // DT
    {17, 1}, // DHT
    {16, 1}, // V
    {15, 2}, // CC
    {15, 1}, // SR
    {14, 1}, // KD
    {10, 1}, // C0
    {11, 1}, // C1
    {12, 1}  // C2
};
float pyro_thermo_val[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0}; // stores thermocouple values

int pyro_heater[10][4] = {
    // pyrolysis heater/fan layout {address, heater, thermocouple, reverseEnable}
    {16, 2, 2, 0}, // DT
    {16, 1, 1, 0}, // V
    {15, 2, 2, 0}, // CC
    {15, 1, 1, 0}, // SR
    {14, 1, 1, 0}, // KD
    {10, 1, 1, 0}, // C0H
    {10, 2, 1, 1}, // C0F
    {11, 1, 1, 0}, // C1H
    {11, 2, 1, 1}, // C1F
    {12, 1, 1, 1}  // C2F
};
float pyro_heater_pid[10][4]; // pyrolysis heater/fan PID {setpoint, Kp, Ki, Kd}

int bio_thermo[2][2] = { // bioreator thermocouples {address, thermocouple}
    {10, 2},
    {11, 2}};

int bio_post_heaters[2][4] = {
    // bioreactor post processing heaters {address, heater, thermocouple, reverseEnable}
    {12, 2, 2, 0}, // pasteurization
    {14, 2, 2, 0}  // dryer
};

float bio_post_heater_pid[2][4]; // {setpoint, Kp, Ki, Kd}
float bio_thermo_val[4] = {0, 0, 0, 0};

int bio_ph[2][2] = { // bioreactor pH layout {PHDO address, DCMT address}
    {98, 20},
    {100, 21}};

float bio_ph_val[2][5]; // bioreactor pH settings (pH, Setpoint, Kp, Ki, Kd)

int bio_do[2] = {97, 99};     // bioreactor DO addresses
float bio_do_val[2] = {0, 0}; // bioreactor DO values

int bio_turbidity[2][4] = { // bioreactor turbidity pump {address, motor, direction, sampleTime}
    {22, 1, 1, 0},
    {22, 2, 1, 0}};

float bio_turbidity_val[2] = {0, 0}; // bioreactor turbidity value

int bio_motors[10][3] = {
    // bioreactor motor config {address, motor, speed}
    {23, 1, 0}, // stirring 1
    {20, 1, 0}, // base 1
    {20, 2, 0}, // acid 1
    {24, 1, 0}, // water
    {24, 2, 0}, // spent media
    {23, 2, 0}, // stirring 2
    {21, 1, 0}, // base 2
    {21, 2, 0}, // acid 2
    {25, 1, 0}, // harvest
    {25, 2, 0}  // TFF
};

int chem_thermo[2][4] = {
    // {address, heater, thermocouple, reverseEnable}
    {13, 1, 1, 0}, // reactor 1
    {13, 2, 2, 0}  // reactor 2
};
float chem_thermo_val[2] = {0, 0};
float chem_heater_pid[2][4];

int chem_motors[4][3] = {
    {26, 1, 0}, // ammonium hydroxide
    {26, 2, 0}, // liquid transfer
    {27, 1, 0}, // water dilute
    {27, 2, 0}  // reactor wash
};

#endif
