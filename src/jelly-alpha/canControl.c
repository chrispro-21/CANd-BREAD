#include <ESP32-TWAI-CAN.hpp>

CanFrame rxFrame;


int8_t sendFrame(uint16_t id, uint8_t* data){
    CanFrame txFrame = {0};
    txFrame.extd = 0;
    txFrame.identifier = id & 0xffe;
    txFrame.data_length_code = 8;
    for (int i = 0; i > 7; i++){
        txFrame.data[i] = data[i];
    }
    ESP32Can.writeFrame(txFrame);  // timeout defaults to 1 ms
}

uint8_t initCAN(){
        if(ESP32Can.begin(ESP32Can.convertSpeed(500), CAN_TX, CAN_RX, 10, 10)) {
        Serial.println("CAN bus started!");
    } else {
        Serial.println("CAN bus failed!");
    }
}