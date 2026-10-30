// imu_tester.cpp

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

Adafruit_MPU6050 mpu;

void setupIMU() {
    Wire.begin();

    if (!mpu.begin()) {
        Serial.println("MPU6050 not found");

        return; // Allow UART testing even when the IMU is not connected.
    }

    Serial.println("MPU6050 connected");
}

void printIMU() {
    sensors_event_t a, g, temp;

    mpu.getEvent(&a, &g, &temp);

    Serial.print("Accel X: ");
    Serial.println(a.acceleration.x);
    Serial.println();
    Serial.print("Accel Y: ");  
    Serial.println(a.acceleration.y);
    Serial.println();
    Serial.print("Accel Z: ");
    Serial.println(a.acceleration.z);
    Serial.println();
    // Serial.println("Gyro X: ");
    // Serial.println(g.gyro.x);
    // Serial.println();
    // Serial.print("Gyro Y: ");   
    // Serial.println(g.gyro.y);
    // Serial.println();
    // Serial.print("Gyro Z: ");
    // Serial.println(g.gyro.z);

    delay(1000);
}
