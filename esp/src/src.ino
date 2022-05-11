#include <Arduino.h>

#include "num.h"
#include "imu.h"
#include "web.h"
#include "peer.h"
#include "timer.h"
#include "button.h"
#include "draw.h"

/* PROGRAM PARAMETERS */
#define UPLOAD_RECV         // Uncomment to upload the receiver code
//#define UPLOAD_LEFT         // Uncomment to upload the left-remote's code

const u8 ADDR_RECV[] = { 0x7C, 0xDF, 0xA1, 0x1A, 0x2F, 0xE6 };
const u8 ADDR_L[] = {};
const u8 ADDR_R[] = {};


#if defined UPLOAD_RECV
Draw draw;
int score = 0;
Timer timer(50);

void setup() {
    // Website is configured for 115200 bps
    Serial.begin(115200);

    // Await serial
    while (!Serial);

    // Place this in ADDR below
    Serial.println(Peer::addr());

    draw.begin();

    // Relay every message received
    Peer::recv(+[](const u8* mac, const u8* buf, int len) {
        Serial.write(buf, len);
    });
}

void loop() {
    while (Serial.available()) {
        i32 hits;
        Serial.read((u8*)&hits, sizeof(i32));
        Serial.printf("Hits: %d\n", hits);
        if(hits>0){
            draw.set_animationNumber(1);
            score+=hits;
        }else{
            draw.set_animationNumber(2);
        }
    }
    if(timer.poll()){
        draw.draw(score);
    }
}

#else
Timer timer(50);        // Serial write timer
Imu imu;                // MPU6050

Peer peer(ADDR_RECV);   // Peer networking
Web http;

Button btn1(45);
Button btn2(39);

vec3 rot;               // Integrated rotation
vec3 vel;               // Integrated velocity
vec3 pos;               // Integrated position
vec3 g;                 // Gravity vector(from calibration)
u64 t;                  // Last time reading(ms)

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
    imu.calibrate(100, 10);
}

void loop() {
    // Get latest IMU readings
    imu.poll();
    if (!t) {
        t = micros();
    }
    // Delta time
    f64 dt = (micros() - t) / 1000000.0;
    t = micros();

    // Integrate(Θ[n] = Θ[n-1] + ω * Δt)
    rot += imu.gyro() * dt;

    // Local -> Global acceleration based off rotation
    vec3 acc = imu.accel()
        .rotate_axis(Y, rot.y)
        .rotate_axis(X, rot.x);
    // Compensate for gravity
    acc -= g;
    // Integrate(v[n] = v[n-1] + a * Δt)
    vel += acc * dt;
    // Integrate(x[n] = x[n-1] + v * Δt)
    pos += vel * dt;

    vec3 a = imu.accel();
    if (abs(a.len() - 10.5) < 0.1) {
        rot.x = asin(a.y / sqrt(a.y*a.y + a.z*a.z)) - (PI / 2.0);
        rot.y = asin(a.x / sqrt(a.x*a.x + a.z*a.z));
        rot.z = 0;
    }
    // Reset button
    if (btn1.poll()) {
        rot = vec3::zero();
    }
    // Serial/Peer write
    if (timer.poll()) {
        // Serial.write((u8*)&rot, sizeof(vec3));
        Serial.printf("(%f, %f, %f)\n", vel.x, vel.y, vel.z);
        peer.send((u8*)&rot, sizeof(vec3));
        vel = vec3::zero();
    }
}
#endif