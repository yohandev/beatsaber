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
public:
    Adafruit_MPU6050 d; // Actual drivers

    vec3 acc;           // Last polled accel
    vec3 gyr;           // Last polled gyro

    vec3 ang_drift;         // Gyroscope bias
    vec3 acc_drift;
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
    void calibrate(u32 n, u32 period = 1, vec3 gravity = vec3()) {
        // Range: high precision
        this->d.setGyroRange(MPU6050_RANGE_250_DEG);
        // Reset bias
        this->ang_drift = vec3::zero();
        this->acc_drift = vec3::zero();
        // Average
        for (i32 i = 0; i < n; i++) {
            this->ang_drift += this->poll().gyr;
            this->acc_drift += this->poll().acc - gravity;
            // Not super precise but it's OK
            delay(period);
        }
        this->ang_drift /= n;
        this->acc_drift /= n;
        // Range: high speed
        this->d.setGyroRange(MPU6050_RANGE_2000_DEG);
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
        return this->acc - this->acc_drift;
    }

    vec3 get_drift() const{
      return this->acc_drift;
    }

    /**
     * Get the last-polled acceleromter readings(m/s^2) without drift compensation
     */
    vec3 accel_raw() const {
      return this->acc;
    }

    /**
     * Get the last-polled gyroscope readings(rad/s)
     */
    vec3 gyro() const {
        return this->gyr - this->ang_drift;
    }
};