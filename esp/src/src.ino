#include <Arduino.h>

#include "timer.h"
#include "imu.h"
#include "num.h"

Timer timer(32);    // Serial write timer
Imu imu;            // MPU6050

vec3 rot;           // Integrate rotation(deg)
u64 ms;             // Last time reading(ms)

void setup() {
    Serial.begin(115200);
    imu.begin();
    imu.calibrate(100);

    // Await serial
    while (!Serial);
}

void loop() {
    // Integrate(Θ[n] = Θ[n-1] + G * Δt)
    rot += imu.poll().gyro() * ((millis() - ms) / 1000.0);
    ms = millis();

    // Serial write
    if (timer.poll()) {
        Serial.write((u8*)&rot, sizeof(vec3));
    }
}