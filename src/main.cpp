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

int8_t sendFrame(uint16_t id, uint8_t dataLen, uint8_t* data){
    CanFrame txFrame = {0};
    txFrame.extd = 0;
    txFrame.identifier = id & 0xffe;
    txFrame.data_length_code = dataLen;

    for (int i = 0; i < dataLen; i++){
        txFrame.data[i] = data[i];
    }

    return ESP32Can.writeFrame(txFrame);  // timeout defaults to 1 ms
}

uint8_t initCAN(){
    // wakey wakey CAN tranciever
    // pinMode(CAN_STDBY, OUTPUT);
    // digitalWrite(CAN_STDBY, 0x01);
    

    if(ESP32Can.begin(ESP32Can.convertSpeed(500), CAN_TX, CAN_RX, 10, 10)) {
        Serial.println("CAN bus started!");
        return 1;
    } else {
        Serial.println("CAN bus failed!");
        return 0;
    }
}
uint32_t lastPOST;
bool isHeThere = 0;
uint32_t freq = 0;

bool isDeviceOnCAN() {
    return isHeThere;
}
// Function to convert 4 bytes to a float
float bytesToFloat(byte* data) {
    float value;
    memcpy(&value, data, sizeof(value));
    return value;
}

// Function to convert Fahrenheit to Celsius
float fahrenheitToCelsius(float f) {
    return (f - 32) * 5.0 / 9.0;
}

// Function to convert Celsius to Fahrenheit
float celsiusToFahrenheit(float c) {
    return (c * 9.0 / 5.0) + 32;
}

void setup()
{
    Serial.begin(115200);

    freq = getCpuFrequencyMhz();
    Serial.printf("CPU Freq = %d Mhz\r\n", freq);
    freq = getXtalFrequencyMhz();
    Serial.printf("XTAL Freq = %d Mhz\r\n", freq);
    freq = getApbFrequency();
    Serial.printf("APB Freq = %d Mhz\r\n", freq);

    initCAN();

    // Start Wi-Fi Access Point
    startWiFi();

    // Serve the root web page
    // God please forgive me. This is what I must do until I get an SD card with a file system
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        String message = "<html><head><title>Canned BREAD Web Interface</title>";
        message += "<style>"
                    "body { font-family: Arial, sans-serif; background-color: #f4f4f9; color: #333; margin: 0; padding: 20px; }"
                    "h1 { color: #4CAF50; }"
                    "h2 { color: #2196F3; }"
                    "p { font-size: 16px; line-height: 1.5; }"
                    "strong { color: #ff5722; }"
                    "ul { list-style-type: none; padding: 0; }"
                    "li { background-color: #fff; margin: 5px 0; padding: 8px; border: 1px solid #ddd; border-radius: 5px; }"
                    "li:nth-child(odd) { background-color: #f9f9f9; }"
                    "ul li span { font-weight: bold; color: #333; }"
                    ".container { max-width: 900px; margin: 0 auto; padding: 20px; background-color: #fff; border-radius: 10px; box-shadow: 0 4px 8px rgba(0, 0, 0, 0.1); }"
                    ".header { margin-bottom: 20px; }"
                    "</style>";
        message += "</head><body>";
        message += "<div class='container'><div class='header'><h1>Canned BREAD Web Interface</h1>";
        message += "<p><strong>Is a device on the CAN BUS?</strong></p>";

        if (isDeviceOnCAN()) {
            message += "<p><strong>Device is present on the CAN bus: True</strong></p>";
            
            // Add information about the last message received
            message += "<h2>Last Message Received:</h2>";
            message += "<p>From: <span>0x" + String(rxFrame.identifier, HEX) + "</span></p>";
            message += "<p>Data:</p><ul>";
            
            // Extract sensor data from the CAN frame
            byte sensorType = rxFrame.data[0];
            byte sensorChannel = rxFrame.data[1];
            byte tempUnit = rxFrame.data[2];  // F or C
            float temperature = bytesToFloat(&rxFrame.data[3]);  // Convert 4 bytes to float
            byte unusedByte = rxFrame.data[7];  // Last byte (unused)

            // Display raw data
            message += "<li>Sensor Type: <span>" + String(sensorType) + "</span></li>";
            message += "<li>Sensor Channel: <span>" + String(sensorChannel) + "</span></li>";
            message += "<li>Temperature Unit: <span>" + (tempUnit == 'F' ? "Fahrenheit" : "Celsius") + "</span></li>";
            message += "<li>Raw Temperature Value: <span>" + String(temperature) + "</span></li>";

            // Convert temperature if necessary
            String tempDisplay = String(temperature);
            if (tempUnit == 'F') {
                float celsiusValue = fahrenheitToCelsius(temperature);
                tempDisplay += " (converted to Celsius: " + String(celsiusValue) + "°C)";
            } else {
                float fahrenheitValue = celsiusToFahrenheit(temperature);
                tempDisplay += " (converted to Fahrenheit: " + String(fahrenheitValue) + "°F)";
            }
            
            message += "<li>Temperature: <span>" + tempDisplay + "</span></li>";
            message += "</ul>";
        } else {
            message += "<p><strong>No device found on the CAN bus.</strong></p>";
        }
        
        message += "</div></div>";
        message += "</body></html>";
        request->send(200, "text/html", message);
    });



    // Start the Webserver
    server.begin();
}


void loop()
{
    char message[8] = "Testing";
    if (millis() - lastPOST > SLICE_DATA_INTERVAL_MS) {
        sendFrame(0x122, 8, (uint8_t*) message);
        lastPOST = millis();
    }
    if(ESP32Can.readFrame(rxFrame, 2000)) {
        Serial.printf("Received frame from: %03X. Data is:", rxFrame.identifier);
        for(int i = 0; i < rxFrame.data_length_code; i++){
            Serial.printf("0x%02X, ", rxFrame.data[i]);
        }
        Serial.println("");
        isHeThere = 1;
    }
    else {
        Serial.println("He not there, big sad :(");
        rxFrame = {0};
        isHeThere = 0;
    }
        
}