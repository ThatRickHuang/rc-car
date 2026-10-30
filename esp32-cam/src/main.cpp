#include <Arduino.h>
#include "web_server.h"

void setup() {
  Serial.begin(115200);
  Serial2.begin(9600, SERIAL_8N1, 3, 40); // ELEGOO ESP32-S3 camera: RX, TX to shield

  delay(1000); // Wait for serial monitor to initialize

  Serial.println("Starting web server...");

  setupWebServer();
}

void loop() {
  handleClientRequests();
}

