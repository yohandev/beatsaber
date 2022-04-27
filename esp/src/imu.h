/**
 * [imu.h] MPU6050 drivers
 */
#pragma once

#include <Arduino.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include <math.h>

#include "num.h"

class Imu {
    Adafruit_MPU6050 d; // Actual drivers

    vec3 acc;           // Last polled accel
    vec3 gyr;           // Last polled gyro

    vec3 drift;         // Gyroscope bias
public:
    /**
     * Attempts to begin the I2C connection and connect to the
     * MPU6050 peripheral on the given pins. Returns `true` if
     * operation was succesful.
     */
    bool begin(i32 sda = -1, i32 scl = -1, u32 freq = 0, u8 timeout = 1) {
        return this->d.begin();
    }

    /**
     * Calibrate the gyroscope drift by averaging `n` readings every
     * `period` ms. The device must be at standstill while this occurs.
     */
    void calibrate(u32 n, u32 period = 1) {
        // Reset bias
        this->drift = vec3();
        // Average
        for (i32 i = 0; i < n; i++) {
            this->drift += this->poll().gyr;
            // Not super precise but it's OK
            delay(period);
        }
        this->drift /= n;
    }

    /**
     * Polls the current accelerometer and gyroscope readings.
     * Returns `this` for function chaining.
     */
    Imu& poll() {
        // Accelerometer
        sensors_event_t a, g, t;

        this->d.getEvent(&a, &g, &t);
        this->acc = {
            .x = a.acceleration.x,
            .y = a.acceleration.y,
            .z = a.acceleration.z,
        };
        this->gyr = {
            .x = g.gyro.x,
            .y = g.gyro.y,
            .z = g.gyro.z,
        };

        return *this;
    }

    /**
     * Get the last-polled accelerometer readings(m/s^2)
     */
    vec3 accel() const {
        return this->acc;
    }

    /**
     * Get the last-polled gyroscope readings(rad/s)
     */
    vec3 gyro() const {
        return this->gyr - this->drift;
    }
};