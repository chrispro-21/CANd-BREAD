#include <ESP32-TWAI-CAN.hpp>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>

#include "FS.h"
#include "SPI.h"
#include <Wire.h>

#include "Config.h"
#include "DataStructures.h"
#include "SliceControl.h"
#include "Storage.h"
#include "WebServer.h"

// Default for ESP32
#define CAN_TX		5
#define CAN_RX		4


const char *ssid = "BREAD-ESP32";
const char *password = "P@ssw0rd123#";

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

void setup() {
    // Setup serial for debbuging.
    Serial.begin(115200);

    initSDCard();

    // It is also safe to use .begin() without .end() as it calls it internally
    if(ESP32Can.begin(ESP32Can.convertSpeed(500), CAN_TX, CAN_RX, 10, 10)) {
        Serial.println("CAN bus started!");
    } else {
        Serial.println("CAN bus failed!");
    }
}

void loop() {

    // You can set custom timeout, default is 1000
    if(ESP32Can.readFrame(rxFrame, 1000)) {
        // Comment out if too many requests 
        Serial.printf("Received frame from: %03X. Data is: %x \r\n", rxFrame.identifier, rxFrame.data);
    }

}