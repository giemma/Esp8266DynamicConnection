#include <Arduino.h>
#include <ESP8266WebServer.h>
#include <WiFiUDP.h>


#include "Portal.h"

Portal* portal = new Portal();

void setup() {
      Serial.begin(115200);
      portal->initialize();
}

void loop() {
  portal->handleClient();

}