#include <Arduino.h>

#include "timer.h"
#include "peer.h"
#include "imu.h"
#include "num.h"

#define RECV // Uncomment this line to upload the receiver code

#if defined RECV
void setup() {
    // Website is configured for 115200 bps
    Serial.begin(115200);

    // Await serial
    while (!Serial);

    // Place this in ADDR below
    Serial.println(Peer::addr());

    // Relay every message received
    Peer::recv(+[](const u8* mac, const u8* buf, int len) {
        Serial.write(buf, len);
    });
}

void loop() { /* It's empty here... */ }
#else
// Remote peer MAC address
const u8 ADDR[] = { 0x7C, 0xDF, 0xA1, 0x0F, 0x07, 0x66 };

Timer timer(32);    // Serial write timer
Imu imu;            // MPU6050

vec3 rg;            // Gyroscope rotation(rad)
u64 ms;             // Last time reading(ms)

Peer peer(ADDR);    // Peer networking

void setup() {
    Serial.begin(115200);
    peer.begin();
    imu.begin();
    imu.calibrate(100);
}

void loop() {
    // Rotation from gyroscope integration(Θ[n] = Θ[n-1] + ω * Δt)
    rg += imu.poll().gyro() * ((millis() - ms) / 1000.0);
    ms = millis();

    // Rotation from accelerometer(gravity down)
    vec3 a = imu.accel();
    vec3 ra = {
        .x = atan2(a.x, sqrt(a.y*a.y + a.z*a.z)),
        .y = atan2(a.y, sqrt(a.x*a.x + a.z*a.z)),
        .z = atan2(sqrt(a.y*a.y + a.x*a.x), a.z),
    };

    // Final rotation estimate(4% accel, 96% gyro)
    //  >> Gravity doesn't drift, will slowly correct that of gyroscope
    //  >> Gyroscope can capture sudden and fine changes in rotation, but drifts 
    vec3 r = (ra * 0.04) + (rg * 0.96);

    // Serial write
    if (timer.poll()) {
        peer.send((u8*)&r, sizeof(vec3));
        Serial.write((u8*)&r, sizeof(vec3));
    }
}
#endif