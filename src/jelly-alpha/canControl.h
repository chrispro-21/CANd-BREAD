#include <stdint.h>
#include <Arduino.h>
#include <ESP32-TWAI-CAN.hpp>

CanFrame rxFrame;

uint8_t initCAN();

int8_t sendFrame(uint16_t id, uint8_t* data);