/**
 * [timer.h] Repeating timer with self-contained state
 */
#pragma once

#include <math.h>
#include <stdint.h>
#include <Arduino.h>

#include "num.h"

/**
 * A repeating timer
 */
class Timer {
    u64 start;          // Start time(µs)
    u64 period;         // Period(µs)
    u64 cycle;          // Cycle incrementor
    u64 delta;          // Delta time(µs)
public:
    /**
     * Starts the timer immediately with the given period(ms)
     */
    Timer(u64 period) : period(period * 1000), cycle(0) {
        this->start = micros();
    }

    /**
     * Ticks the timer and returns `true` if a cycle has elapsed
     */
    bool poll() {
        if (micros() - this->start < this->period) {
            return false;
        }
        this->delta = micros() - this->start;
        this->start = micros();
        this->cycle++;

        return true;
    }

    /**
     * Get the delta time(in seconds) between this cycle and
     * the last
     */
    f64 dt() const {
        return ((float)this->delta) / 1000000.0;
    }
    /**
     * Get the frames per second(naive implementation, no running average)
     */
    f64 fps() const {
        return 1 / max(this->dt(), 0.0001);
    }
};