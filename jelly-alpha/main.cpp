#include "Config.h"
#include <ESPAsyncWebServer.h>
#include <ESP32-TWAI-CAN.hpp>

AsyncWebServer server(80);

// WiFi credentials

CanFrame txFrame,rxFrame;
void startWiFi() {
    WiFi.softAP(ssid, password);
    IPAddress myIP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(myIP);
    Serial.println("Server started");
}

int8_t sendFrame(uint16_t id, uint8_t* data){
    CanFrame txFrame = {0};
    txFrame.extd = 0;
    txFrame.identifier = id & 0xffe;
    txFrame.data_length_code = 8;
    for (int i = 0; i > 7; i++){
        txFrame.data[i] = data[i];
    }
    return ESP32Can.writeFrame(txFrame);  // timeout defaults to 1 ms
}

uint8_t initCAN(){
        if(ESP32Can.begin(ESP32Can.convertSpeed(500), CAN_TX, CAN_RX, 10, 10)) {
        Serial.println("CAN bus started!");
    } else {
        Serial.println("CAN bus failed!");
    }
}
uint32_t lastPOST;
bool isHeThere = 0;
bool isDeviceOnCAN() {
    return isHeThere;
}

void setup()
{
    Serial.begin(115200);

    initCAN();

    // Start Wi-Fi Access Point
    startWiFi();


    // Serve the root web page
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        String message = "Device Status: " + String(isDeviceOnCAN() ? "ON" : "OFF");
        request->send(200, "text/plain", message);
    });
    // Start the Webserver
    server.begin();
}


void loop()
{
    char message[8] = "Hello?";
    if (millis() - lastPOST > SLICE_DATA_INTERVAL_MS) {
        sendFrame(123, (uint8_t*) message);
        if(ESP32Can.readFrame(rxFrame, 1000)) {
            Serial.printf("Received frame from: %03X. Data is: %x \r\n", rxFrame.identifier, rxFrame.data);
            isHeThere = 1;
        }
        else {
            isHeThere = 0;
        }
        lastPOST = millis();
    }
}