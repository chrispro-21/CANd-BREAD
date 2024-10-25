#include "web.h"
#include "Storage.h"
#include "SliceControl.h"
#include "DataStructures.h"

#include "FS.h"
#include "SPI.h"
#include <Wire.h>
#include <ESP32Time.h>

ESP32Time rtc;


AsyncWebServer server(80);
AsyncEventSource events("/events");

// WiFi credentials
const char *ssid = "CAN'd-BREAD";
const char *password = "P@ssw0rd123#";

void startWiFi() {
    WiFi.softAP(ssid, password);
    IPAddress myIP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(myIP);
    Serial.println("Server started");
}

void setupWebServer() {


  // send html data to server
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(SD, "/index.html", "text/html"); });

  // load data from slice form

  /*
  Pyrolysis Commands:
    ps1:  pyrolysis setpoint 1
    pp1:  pyrolysis Kp 1
    pi1:  pyrolysis Ki 1
    pd1:  pyrolysis Kd 1

  Bioreactor Commands:
    bs1:  bioreactor setpoint 1
    bp1:  bioreactor Kp 1
    bi1:  bioreactor Ki 1
    bd1:  bioreactor Kd 1
    bm1:  bioreactor motor 1
    bt1:  bioreactor turbidity sample time 1

  Chem Decon Commands:
    cm1:  chemreactor motor 1 (pump)
    cs1:  chemreactor setpoint 1
    cp1:  chemreactor Kp 1
    ci1:  chemreactor Ki 1
    cd1:  chemreactor Kd 1
  */

  server.on("/form-submit", HTTP_POST, [](AsyncWebServerRequest *request)
            {
    int params = request->params();
    for (int i = 0; i < params; i++) {
      AsyncWebParameter* p = request->getParam(i);
      Serial.printf("POST[%s]: %s\n", p->name().c_str(), p->value().c_str());
      String postName = p->name().c_str();
      String postValue = p->value().c_str();
      uint8_t index = postName.substring(2).toInt() - 1;

      if(postName.charAt(0) == 'p') {   // pyrolysis reactor
        switch(postName.charAt(1)) {
          case 's': //setpoint
            pyro_heater_pid[index][0] = postValue.toFloat();
            RLHTCommandSetpoint(pyro_heater[index][0], pyro_heater[index][1], pyro_heater_pid[index][0], pyro_heater[index][2], pyro_heater[index][3]);
            break;
          case 'p': //Kp
            pyro_heater_pid[index][1] = postValue.toFloat();
            break;
          case 'i': //Ki
            pyro_heater_pid[index][2] = postValue.toFloat();
            break;
          case 'd': //Kd
            pyro_heater_pid[index][3] = postValue.toFloat();
            RLHTCommandPID(pyro_heater[index][0], pyro_heater[index][1], pyro_heater_pid[index][1], pyro_heater_pid[index][2], pyro_heater_pid[index][3]);
            break;
        }
        //float converted into 4 bytes
      }else if(postName.charAt(0) == 'b') {
        uint8_t index = postName.substring(2).toInt() - 1;
        switch(postName.charAt(1)) {
          case 's': //setpoint
            bio_ph_val[index][1] = postValue.toFloat();
            DCMTCommandPH(bio_ph[index][1], bio_ph_val[index][1], bio_ph_val[index][0]);
            break;
          case 'p': //Kp
            bio_ph_val[index][2] = postValue.toFloat();
            //send command to BREAD here:
            break;
          case 'i': //Ki
            bio_ph_val[index][3] = postValue.toFloat();
            //send command to BREAD here:
            break;
          case 'd': //Kd
            bio_ph_val[index][2] = postValue.toFloat();
            Serial.println("Sending pH PID");
            DCMTCommandPHPID(bio_ph[index][1], bio_ph_val[index][2], bio_ph_val[index][3], bio_ph_val[index][4]);
            break;
          case 'm':
            bio_motors[index][2] = postValue.toInt();
            DCMTCommandSpeed(bio_motors[index][0], bio_motors[index][1], bio_motors[index][2], 1);
            break;
          case 't':
            bio_turbidity[index][3] = postValue.toInt();  // sample time
            DCMTCommandTurbidity(bio_turbidity[index][0], bio_turbidity[index][1], bio_turbidity[index][2], 1, bio_turbidity[index][3]);
            break;
        }
      } else if(postName.charAt(0) == 'c') {
        switch(postName.charAt(1)) {
          case 's': //setpoint
            chem_heater_pid[index][0] = postValue.toFloat();
            RLHTCommandSetpoint(chem_thermo[index][0], chem_thermo[index][1], chem_heater_pid[index][0], chem_thermo[index][2], chem_thermo[index][3]);
            //send command to BREAD here:
            break;
          case 'p': //Kp
            chem_heater_pid[index][1] = postValue.toFloat();
            //send command to BREAD here:
            break;
          case 'i': //Ki
            chem_heater_pid[index][2] = postValue.toFloat();
            //send command to BREAD here:
            break;
          case 'd': //Kd
            chem_heater_pid[index][3] = postValue.toFloat();
            RLHTCommandPID(chem_thermo[index][0], chem_thermo[index][1], chem_heater_pid[index][1], chem_heater_pid[index][2], chem_heater_pid[index][3]);
            break;
          case 'm':
            chem_motors[index][2] = postValue.toInt();
            DCMTCommandSpeed(chem_motors[index][0], chem_motors[index][1], chem_motors[index][2], 1);
            break;
        }
      }else if(postName.charAt(0) == 'P') {   // bioreactor post processing
        uint8_t index = postName.substring(2).toInt() - 1;
        switch(postName.charAt(1)) {
          case 's': //setpoint
            bio_post_heater_pid[index][0] = postValue.toFloat();
            RLHTCommandSetpoint(bio_post_heaters[index][0], bio_post_heaters[index][1], bio_post_heater_pid[index][0], bio_post_heaters[index][2], bio_post_heaters[index][3]);
            break;
          case 'p': //Kp
            bio_post_heater_pid[index][1] = postValue.toFloat();
            //send command to BREAD here:
            break;
          case 'i': //Ki
            bio_post_heater_pid[index][2] = postValue.toFloat();
            //send command to BREAD here:
            break;
          case 'd': //Kd
            bio_post_heater_pid[index][3] = postValue.toFloat();
            Serial.println("Sending pH PID");
            RLHTCommandPID(bio_post_heaters[index][0], bio_post_heaters[index][1], bio_post_heater_pid[index][1], bio_post_heater_pid[index][2], bio_post_heater_pid[index][3]);
            break;
        }
      }
    } });

  // load any non-form POST request body and url
  server.onRequestBody([](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
                       {
    Serial.println(request->url());
    if (request->url() == "/all-data") {
      String s = "";
      for(size_t i=0; i<len; i++) {
        s += char(data[i]);
      }
      Serial.println(s);
    }if(request->url() == "/get-variables") {
      //set time on esp32 rtc
      String s = "";
      for(size_t i=0; i<len; i++) {
        s += char(data[i]);
      }
      
      uint16_t timeDate[6];
      for(uint8_t x = 0; x < 5; x++) {
        uint8_t nextComma = s.indexOf(',');
        timeDate[x] = s.substring(0, nextComma).toInt();
        s = s.substring(nextComma+1);
      }
      timeDate[5] = s.toInt();
      rtc.setTime(timeDate[0], timeDate[1], timeDate[2], timeDate[3], timeDate[4], timeDate[5]);

      //how variables are sent to the server (I should probably use a more structured format like JSON)
      String toServer = "";
      if(logging) {
        toServer += "logging-";
      } else {
        toServer += "not logging-";
      }
           
      for(uint8_t x = 0; x < 10; x++) {
        toServer += (
          String(pyro_heater_pid[x][0]) + "|" +   // setpoint
          String(pyro_heater_pid[x][1]) + "|" +          // Kp
          String(pyro_heater_pid[x][2]) + "|" +          // Ki
          String(pyro_heater_pid[x][3]) + ","            // Kd
        );
      }
      toServer.remove(toServer.length()-1);
      toServer += '-';

      for(uint8_t x = 0; x < 2; x++) {
        toServer += (
          String(bio_ph_val[x][1]) + "|" +    // setpoint
          String(bio_ph_val[x][2]) + "|" +    // Kp
          String(bio_ph_val[x][3]) + "|" +    // Ki
          String(bio_ph_val[x][4]) + ","      // Kd
        );
      }
      toServer.remove(toServer.length()-1);
      toServer += '-';
      toServer += String(bio_turbidity[0][3]) + "," + String(bio_turbidity[1][3]);
      toServer += '-';

      for(uint8_t x = 0; x < 2; x++) {
        toServer += (
          String(bio_post_heater_pid[x][0]) + "|" +    // setpoint
          String(bio_post_heater_pid[x][1]) + "|" +    // Kp
          String(bio_post_heater_pid[x][2]) + "|" +    // Ki
          String(bio_post_heater_pid[x][3]) + ","      // Kd
        );
      }
      toServer.remove(toServer.length()-1);
      toServer += '-';
      
      for(uint8_t x = 0; x < 10; x++) {
        toServer += String(bio_motors[x][2]) + ",";   // motor speed
      }
      toServer.remove(toServer.length()-1);
      toServer += '-';

      for(uint8_t x = 0; x < 4; x++) {
        toServer += String(chem_motors[x][2]) + ",";  // motor speed
      }
      toServer.remove(toServer.length()-1);
      toServer += '-';

      for(uint8_t x = 0; x < 2; x++) {
        toServer += (
          String(chem_heater_pid[x][0]) + "|" +    // setpoint
          String(chem_heater_pid[x][1]) + "|" +    // Kp
          String(chem_heater_pid[x][2]) + "|" +    // Ki
          String(chem_heater_pid[x][3]) + ","      // Kd
        );
      }
      toServer.remove(toServer.length()-1);
      
      request->send(200, "text/plain", toServer);
    } if(request->url() == "/logging") {
      Serial.println("log to SD card");
      logging = true;
    } if(request->url() == "/not-logging") {
      Serial.println("stop log to SD card");
      logging = false;
    } if(request->url() == "/estop-on") {
      Serial.println("estop on");
      digitalWrite(ESTOP, HIGH);
    } if(request->url() == "/estop-off") {
      Serial.println("estop off");
      digitalWrite(ESTOP, LOW);
    } else if(request->url() == "/delete-pyrolysis") {
      writeFile(SD, "/pyrolysis-data.csv", "Date and Time,Dissolution Tank,Dissolution Heating Tape,Valve,Char Chamber,Secondary Reactor,Knockout Drum,Condenser 0,Condenser 1,Condenser 2");
    } else if(request->url() == "/delete-bioreactor") {
      writeFile(SD, "/bioreactor-data.csv", "Date and Time,Thermocouple 1,pH Sensor 1,Dissolved Oxygen 1,Turbidity 1,Thermocouple 2, pH Sensor 2, Dissolved Oxygen 2,Turbidity 2,Pasteurization,Dryer");
    } else if(request->url() == "/delete-chemreactor") {
      writeFile(SD, "/chemreactor-data.csv", "Date and Time,Reactor 1,Reactor 2");
    } });

  // Handle Web Server Events
  events.onConnect([](AsyncEventSourceClient *client)
                   {
    if(client->lastId()){
      Serial.printf("Client reconnected! Last message ID that it got is: %u\n", client->lastId());
    }
    // send event with message "hello!", id current millis
    // and set reconnect delay to 1 second
    client->send("hello!", NULL, millis(), 10000); });
  server.addHandler(&events);

  server.serveStatic("/", SD, "/");
  server.begin();

}

void handleRequests() {
    server.onRequestBody([](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
        if (request->url() == "/all-data") {
            handleAllDataRequest(data, len);
        } else if (request->url() == "/get-variables") {
            handleGetVariablesRequest(data, len);
        } else if (request->url() == "/logging") {
            Serial.println("log to SD card");
            logging = true;
        } else if (request->url() == "/not-logging") {
            Serial.println("stop log to SD card");
            logging = false;
        } else if (request->url() == "/estop-on") {
            Serial.println("estop on");
            digitalWrite(ESTOP, HIGH);
        } else if (request->url() == "/estop-off") {
            Serial.println("estop off");
            digitalWrite(ESTOP, LOW);
        } else if (request->url() == "/delete-pyrolysis") {
            writeFile(SD, "/pyrolysis-data.csv", "Date and Time,Dissolution Tank,Dissolution Heating Tape,Valve,Char Chamber,Secondary Reactor,Knockout Drum,Condenser 0,Condenser 1,Condenser 2");
        } else if (request->url() == "/delete-bioreactor") {
            writeFile(SD, "/bioreactor-data.csv", "Date and Time,Thermocouple 1,pH Sensor 1,Dissolved Oxygen 1,Turbidity 1,Thermocouple 2, pH Sensor 2, Dissolved Oxygen 2,Turbidity 2,Pasteurization,Dryer");
        } else if (request->url() == "/delete-chemreactor") {
            writeFile(SD, "/chemreactor-data.csv", "Date and Time,Reactor 1,Reactor 2");
        }
    });
}

// Helper functions for handling specific types of requests
void handlePyrolysisCommands(String postName, String postValue, uint8_t index) {
    switch (postName.charAt(1)) {
        case 's': // Setpoint
            pyro_heater_pid[index][0] = postValue.toFloat();
            RLHTCommandSetpoint(pyro_heater[index][0], pyro_heater[index][1], pyro_heater_pid[index][0], pyro_heater[index][2], pyro_heater[index][3]);
            break;
        case 'p': // Kp
            pyro_heater_pid[index][1] = postValue.toFloat();
            break;
        case 'i': // Ki
            pyro_heater_pid[index][2] = postValue.toFloat();
            break;
        case 'd': // Kd
            pyro_heater_pid[index][3] = postValue.toFloat();
            RLHTCommandPID(pyro_heater[index][0], pyro_heater[index][1], pyro_heater_pid[index][1], pyro_heater_pid[index][2], pyro_heater_pid[index][3]);
            break;
    }
}

void handleBioreactorCommands(String postName, String postValue, uint8_t index) {
    switch (postName.charAt(1)) {
        case 's': // Setpoint
            bio_ph_val[index][1] = postValue.toFloat();
            DCMTCommandPH(bio_ph[index][1], bio_ph_val[index][1], bio_ph_val[index][0]);
            break;
        case 'p': // Kp
            bio_ph_val[index][2] = postValue.toFloat();
            break;
        case 'i': // Ki
            bio_ph_val[index][3] = postValue.toFloat();
            break;
        case 'd': // Kd
            bio_ph_val[index][4] = postValue.toFloat();
            DCMTCommandPHPID(bio_ph[index][1], bio_ph_val[index][2], bio_ph_val[index][3], bio_ph_val[index][4]);
            break;
        case 'm': // Motor speed
            bio_motors[index][2] = postValue.toInt();
            DCMTCommandSpeed(bio_motors[index][0], bio_motors[index][1], bio_motors[index][2], 1);
            break;
        case 't': // Turbidity sample time
            bio_turbidity[index][3] = postValue.toInt();
            DCMTCommandTurbidity(bio_turbidity[index][0], bio_turbidity[index][1], bio_turbidity[index][2], 1, bio_turbidity[index][3]);
            break;
    }
}

void handleChemReactorCommands(String postName, String postValue, uint8_t index) {
    switch (postName.charAt(1)) {
        case 's': // Setpoint
            chem_heater_pid[index][0] = postValue.toFloat();
            RLHTCommandSetpoint(chem_thermo[index][0], chem_thermo[index][1], chem_heater_pid[index][0], chem_thermo[index][2], chem_thermo[index][3]);
            break;
        case 'p': // Kp
            chem_heater_pid[index][1] = postValue.toFloat();
            break;
        case 'i': // Ki
            chem_heater_pid[index][2] = postValue.toFloat();
            break;
        case 'd': // Kd
            chem_heater_pid[index][3] = postValue.toFloat();
            RLHTCommandPID(chem_thermo[index][0], chem_thermo[index][1], chem_heater_pid[index][1], chem_heater_pid[index][2], chem_heater_pid[index][3]);
            break;
        case 'm': // Motor speed
            chem_motors[index][2] = postValue.toInt();
            DCMTCommandSpeed(chem_motors[index][0], chem_motors[index][1], chem_motors[index][2], 1);
            break;
    }
}

void handleBioreactorPostProcessingCommands(String postName, String postValue, uint8_t index) {
    switch (postName.charAt(1)) {
        case 's': // Setpoint
            bio_post_heater_pid[index][0] = postValue.toFloat();
            RLHTCommandSetpoint(bio_post_heaters[index][0], bio_post_heaters[index][1], bio_post_heater_pid[index][0], bio_post_heaters[index][2], bio_post_heaters[index][3]);
            break;
        case 'p': // Kp
            bio_post_heater_pid[index][1] = postValue.toFloat();
            break;
        case 'i': // Ki
            bio_post_heater_pid[index][2] = postValue.toFloat();
            break;
        case 'd': // Kd
            bio_post_heater_pid[index][3] = postValue.toFloat();
            RLHTCommandPID(bio_post_heaters[index][0], bio_post_heaters[index][1], bio_post_heater_pid[index][1], bio_post_heater_pid[index][2], bio_post_heater_pid[index][3]);
            break;
    }
}

void handleAllDataRequest(uint8_t *data, size_t len) {
    String s = "";
    for (size_t i = 0; i < len; i++) {
        s += char(data[i]);
    }
    Serial.println(s);
}

void handleGetVariablesRequest(uint8_t *data, size_t len) {
    String s = "";
    for (size_t i = 0; i < len; i++) {
        s += char(data[i]);
    }
    uint16_t timeDate[6];
    for (uint8_t x = 0; x < 5; x++) {
        uint8_t nextComma = s.indexOf(',');
        timeDate[x] = s.substring(0, nextComma).toInt();
        s = s.substring(nextComma + 1);
    }
    timeDate[5] = s.toInt();
    rtc.setTime(timeDate[0], timeDate[1], timeDate[2], timeDate[3], timeDate[4], timeDate[5]);
}

