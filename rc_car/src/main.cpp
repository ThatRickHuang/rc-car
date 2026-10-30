// main.cpp

#include <Arduino.h>
#include "web_drive.h"

char incoming[32];
byte length = 0;
bool discardLine = false;
unsigned long lastValidCommand = 0;
String currentCommand = "stop";

bool validCommand(const String& command) {
    return command == "stop" || command == "forward" || command == "backward" ||
        command == "pivot_left" || command == "pivot_right" ||
        command == "forward_left" || command == "forward_right" ||
        command == "backward_left" || command == "backward_right";
}

void applyCommand(const String& command) {
    if (command != currentCommand) {
        drive(command);
        currentCommand = command;
    }
}

void setup() {
    setupMotors(); // Configure motor pins and enable the driver.
    Serial.begin(9600); // Shield UART: D0 RX, D1 TX; shared with USB monitor
    Serial.println("UART ready: set shield switch to CAM");
}

void loop() {
    // Read without waiting, so an incomplete message cannot delay stopping.
    byte budget = 64;
    while (Serial.available() && budget--) {
        char c = Serial.read();
        if (c == '\r') continue;
        if (c == '\n') {
            incoming[length] = '\0';
            String command(incoming);
            if (!discardLine && validCommand(command)) {
                lastValidCommand = millis();
                applyCommand(command);
            } else {
                applyCommand("stop");
            }
            length = 0;
            discardLine = false;
        } else if (!discardLine) {
            if (length < sizeof(incoming) - 1) incoming[length++] = c;
            else discardLine = true;
        }
    }
    if (millis() - lastValidCommand >= 700) {
        applyCommand("stop");
    }
}
