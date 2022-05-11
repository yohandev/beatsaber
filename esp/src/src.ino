#include <Arduino.h>

#include "num.h"
#include "imu.h"
#include "web.h"
#include "peer.h"
#include "timer.h"
#include "button.h"

/* PROGRAM PARAMETERS */
// #define UPLOAD_RECV         // Uncomment to upload the receiver code
#define UPLOAD_LEFT         // Uncomment to upload the left-remote's code

const u8 ADDR_RECV[] = { 0x60, 0x55, 0xF9, 0xD9, 0xD7, 0x2A };
const u8 ADDR_L[] = { 0x60, 0x55, 0xF9, 0xD9, 0xD7, 0x02 };
const u8 ADDR_R[] = { 0x7C, 0xDF, 0xA1, 0x1A, 0x2F, 0xE6 };

#if defined UPLOAD_RECV
void setup() {
    // Website is configured for 115200 bps
    Serial.begin(115200);

    // Await serial
    while (!Serial);

    // Place this in ADDR_RECV above
    Serial.println(Peer::addr());

    // Relay every message received
    Peer::recv(+[](const u8* mac, const u8* buf, int len) {
        struct {
            f64 ctrl;
            vec3 rot;
        } packet;
        // Left(0) or right(1)
        packet.ctrl = (mac[0] == ADDR_L[0]) ? 0.0 : 999.0;
        // Rotation
        memcpy(&packet.rot, buf, len);
        // Write
        Serial.write((u8*)&packet, sizeof(packet));
    });
}

void loop() {
    while (Serial.available()) {
        i32 score;
        Serial.read((u8*)&score, sizeof(i32));
        Serial.printf("Score: %d\n", score);
    }
}

#else
Timer t20hz(50);        // Serial write timer
Timer t184hz(1000/184); // IMU read timer

Imu imu;                // MPU6050
vec3 rot;               // Integrated rotation

Peer peer(ADDR_RECV);   // Peer networking
Web http;

Button btn1(45);        // Reset button
Button btn2(39);


void setup() {
    // Website is configured for 115200 bps
    Serial.begin(115200);

    // Await serial
    while (!Serial);

    // Connect to WiFi
    if (!http.connect("MIT GUEST", "")) {
        Serial.println("Could not connect to the internet!");
        ESP.restart();
    }
    // Connect to receiver ESP32
    if (!peer.begin()) {
        Serial.println("Could not connect to peer!");
    }
    // Initialize IMU
    if (!imu.begin()) {
        Serial.println("Could not connected to MPU6050!");
        ESP.restart();
    }
    // Roughly matches timer above
    imu.d.setFilterBandwidth(MPU6050_BAND_184_HZ);
    // Must be layed flat and at rest
    imu.calibrate(100, 10);

    // Place this in ADDR_L or ADDR_R above
    Serial.println(Peer::addr());
}

void loop() {
    // Serial/Peer write
    if (t20hz.poll()) {
        // Serial.write((u8*)&rot, sizeof(vec3));
        peer.send((u8*)&rot, sizeof(vec3));
    }

    // IMU readings
    if (t184hz.poll()) {
        // Integrate(Θ[n] = Θ[n-1] + ω * Δt)
        rot += imu.poll().gyro() * t184hz.dt();
    }

    // Reset button
    if (btn1.poll() == 1) {
        rot = vec3::zero();
        #if defined UPLOAD_LEFT
        if (!http.post("608dev-2.net", "sandbox/sc/team27/user_esp_server.py", "op=select")
        || !http.post("608dev-2.net", "sandbox/sc/team27/user_esp_server.py", "op=reset")) {
            Serial.println("Post request failed!");
        }
        #endif
    }
    // GUI button
    if (btn2.poll() == 1) {
        #if defined UPLOAD_LEFT
        if (!http.post("608dev-2.net", "sandbox/sc/team27/user_esp_server.py", "op=right")) {
            Serial.println("Post request failed!");
        }
        #else
        if (!http.post("608dev-2.net", "sandbox/sc/team27/user_esp_server.py", "op=left")) {
            Serial.println("Post request failed!");
        }
        #endif
    }
}
#endif