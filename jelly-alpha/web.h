#ifndef WEBSERVER_H
#define WEBSERVER_H

#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>

extern AsyncWebServer server;
extern AsyncEventSource events;

// Function declarations for web server setup
void setupWebServer();
void startWiFi();
void handleRequests();

#endif
