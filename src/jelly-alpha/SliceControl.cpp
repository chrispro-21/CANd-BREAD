#include "SliceControl.h"
#include <Wire.h>
#include <Arduino.h>
#include "Config.h"

extern DCMT_t DCMT[NUM_DCMT_SLICES];
extern RLHT_t RLHT[NUM_RLHT_SLICES];
extern PHDO_t PHDO[NUM_PHDO_SLICES];

void initSlices()
{
    for (int i = 0; i < NUM_DCMT_SLICES; i++)
    {
        DCMT[i].address = 20 + i; // Assign addresses 20-26
    }
    for (int i = 0; i < NUM_RLHT_SLICES; i++)
    {
        RLHT[i].address = 10 + i; // Assign addresses 10-15
    }
    PHDO[0].addressDO = 97;
    PHDO[0].addressPH = 98;
    PHDO[1].addressDO = 99;
    PHDO[1].addressPH = 100;
}

void RLHTRequestThermo(int address, float *t1, float *t2)
{
    bool data_received = false;
    byte in_char;
    char in_data[20];
    FLOATUNION_t thermo1, thermo2;

    thermo1.number = 0;
    thermo2.number = 0;

    Wire.requestFrom(address, 8, 1);
    int i = 0;
    while (Wire.available())
    {
        data_received = true;
        in_char = Wire.read();
        in_data[i] = in_char;
        i++;
    }

    if (data_received)
    {
        for (int x = 0; x < 4; x++)
        {
            thermo1.bytes[x] = in_data[x];
            thermo2.bytes[x] = in_data[x + 4];
        }
    }

    *t1 = thermo1.number;
    *t2 = thermo2.number;
}

void RLHTCommandSetpoint(int address, byte heater, float heatSetpoint, byte thermocouple, bool enableReverse)
{
    FLOATUNION_t setpoint;
    setpoint.number = heatSetpoint;

    Wire.beginTransmission(address);
    Wire.write('H');
    Wire.write(heater);
    for (int i = 0; i < 4; i++)
    {
        Wire.write(setpoint.bytes[i]);
    }
    Wire.write(thermocouple);
    Wire.write(enableReverse);
    Wire.endTransmission();
}

void RLHTCommandPID(int address, byte heater, float Kp_set, float Ki_set, float Kd_set)
{
    FLOATUNION_t Kp, Ki, Kd;
    Kp.number = Kp_set;
    Ki.number = Ki_set;
    Kd.number = Kd_set;

    Wire.beginTransmission(address);
    Wire.write('P');
    Wire.write(heater);
    for (int i = 0; i < 4; i++)
    {
        Wire.write(Kp.bytes[i]);
    }
    for (int i = 0; i < 4; i++)
    {
        Wire.write(Ki.bytes[i]);
    }
    for (int i = 0; i < 4; i++)
    {
        Wire.write(Kd.bytes[i]);
    }
    Wire.endTransmission();
}

void DCMTCommandSpeed(int address, byte motor, int speed_set, bool enable)
{
    INTUNION_t speed;
    speed.number = speed_set;

    Wire.beginTransmission(address);
    Wire.write('M');
    Wire.write(motor);
    for (int i = 0; i < 4; i++)
    {
        Wire.write(speed.bytes[i]);
    }
    Wire.write(enable);
    Wire.endTransmission();
}

void DCMTCommandPH(int address, float pHSetpoint_set, float currentPH_set)
{
    FLOATUNION_t pHSetpoint, currentPH;
    pHSetpoint.number = pHSetpoint_set;
    currentPH.number = currentPH_set;

    Wire.beginTransmission(address);
    Wire.write('p');
    for (int i = 0; i < 4; i++)
    {
        Wire.write(currentPH.bytes[i]);
    }
    for (int i = 0; i < 4; i++)
    {
        Wire.write(pHSetpoint.bytes[i]);
    }
    Wire.endTransmission();
}

void DCMTCommandPHPID(int address, float Kp_set, float Ki_set, float Kd_set)
{
    FLOATUNION_t Kp, Ki, Kd;
    Kp.number = Kp_set;
    Ki.number = Ki_set;
    Kd.number = Kd_set;

    Wire.beginTransmission(address);
    Wire.write('P');
    for (int i = 0; i < 4; i++)
    {
        Wire.write(Kp.bytes[i]);
    }
    for (int i = 0; i < 4; i++)
    {
        Wire.write(Ki.bytes[i]);
    }
    for (int i = 0; i < 4; i++)
    {
        Wire.write(Kd.bytes[i]);
    }
    Wire.endTransmission();
}

void DCMTCommandTurbidity(int address, byte motor, byte direction, bool enable, int sample_period)
{
    INTUNION_t sample_p;
    sample_p.number = sample_period;

    Wire.beginTransmission(address);
    Wire.write('T');
    Wire.write(motor);
    Wire.write(direction);
    Wire.write(enable);
    for (int i = 0; i < 4; i++)
    {
        Wire.write(sample_p.bytes[i]);
    }
    Wire.endTransmission();
}

void DCMTRequestTurbidity(int address, float *turbidity1, float *turbidity2)
{
    bool data_received = false;
    byte in_char;
    char in_data[20];
    FLOATUNION_t turb1, turb2;

    turb1.number = 0;
    turb2.number = 0;

    Wire.requestFrom(address, 10, 1);
    int i = 0;
    while (Wire.available())
    {
        data_received = true;
        in_char = Wire.read();
        in_data[i] = in_char;
        i++;
    }

    if (data_received)
    {
        for (int x = 0; x < 4; x++)
        {
            turb1.bytes[x] = in_data[x];
            turb2.bytes[x] = in_data[x + 4];
        }
    }

    *turbidity1 = turb1.number;
    *turbidity2 = turb2.number;
}

void PHDOCommand(int address, const char *commandData)
{
    Wire.beginTransmission(address);
    Wire.write(commandData);
    Wire.endTransmission();
}

float PHDORequest(int address)
{
    byte in_char;
    char in_data[20];
    FLOATUNION_t data;

    Wire.requestFrom(address, 20, 1);

    byte code = Wire.read();

    switch (code)
    {
    case 1:
        Serial.println("Success");
        break;
    case 2:
        Serial.println("Failed");
        break;
    case 254:
        Serial.println("Pending");
        break;
    case 255:
        Serial.println("No Data");
        return 0;
    default:
        Serial.println("Unknown code");
        return 0;
    }

    int i = 0;
    while (Wire.available() && code == 1)
    {
        in_char = Wire.read();
        in_data[i] = in_char;
        i++;
        if (in_char == 0)
        {
            break;
        }
    }

    return atof(in_data);
}
