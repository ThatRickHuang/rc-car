#include "web_drive.h"

void setupMotors() {
    pinMode(7, OUTPUT); // Motor A direction
    pinMode(5, OUTPUT); // Motor A speed (PWM)
    pinMode(8, OUTPUT); // Motor B direction
    pinMode(6, OUTPUT); // Motor B speed (PWM)
    pinMode(3, OUTPUT); //Motor driver enable pin

    analogWrite(5, 0);      // Start stopped
    analogWrite(6, 0);      // Start stopped
    digitalWrite(3, HIGH); // Enable driver
}

void drive(const String& command) {
    if (command == "stop") {
        analogWrite(5, 0); // Stop
        analogWrite(6, 0); // Stop
    } else if (command == "forward") {
        digitalWrite(7, HIGH); // Forward
        digitalWrite(8, HIGH); // Forward
        analogWrite(5, 220);   // Reduced driving speed
        analogWrite(6, 220);   // Reduced driving speed
    } else if (command == "backward") {
        digitalWrite(7, LOW);  // Backward
        digitalWrite(8, LOW);  // Backward
        analogWrite(5, 220);   // Reduced driving speed
        analogWrite(6, 220);   // Reduced driving speed
    } else if (command == "pivot_left") {
        digitalWrite(7, HIGH); // Right forward
        digitalWrite(8, LOW);  // Left backward
        analogWrite(5, 220);   // Reduced driving speed
        analogWrite(6, 220);   // Reduced driving speed
    } else if (command == "pivot_right") {
        digitalWrite(7, LOW);  // Right backward
        digitalWrite(8, HIGH); // Left forward
        analogWrite(5, 220);   // Reduced driving speed
        analogWrite(6, 220);   // Reduced driving speed
    } else if (command == "forward_left") {
        digitalWrite(7, HIGH); // Forward
        digitalWrite(8, HIGH);
        analogWrite(5, 220); // Right faster
        analogWrite(6, 110);
    } else if (command == "forward_right") {
        digitalWrite(7, HIGH); // Forward
        digitalWrite(8, HIGH);
        analogWrite(5, 110);   // Half speed
        analogWrite(6, 220);
    } else if (command == "backward_left") {
        digitalWrite(7, LOW);  // Backward
        digitalWrite(8, LOW);
        analogWrite(5, 110); // Reverse arc toward the left
        analogWrite(6, 220);
    } else if (command == "backward_right") {
        digitalWrite(7, LOW);  // Backward
        digitalWrite(8, LOW);
        analogWrite(5, 220);
        analogWrite(6, 110);
    }
    Serial.print("Command received (in drive function): ");
    Serial.println(command);



}

