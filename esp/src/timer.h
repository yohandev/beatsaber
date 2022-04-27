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
    u64 start;          // Start time(ms)
    u64 period;         // Period(ms)
    u64 cycle;          // Cycle incrementor
    u64 delta;          // Delta time(ms)
public:
    /**
     * Starts the timer immediately with the given period(ms)
     */
    Timer(u64 period) : period(period), cycle(0) {
        this->start = millis();
    }

    /**
     * Ticks the timer and returns `true` if a cycle has elapsed
     */
    bool poll() {
        if (millis() - this->start < this->period) {
            return false;
        }
        this->delta = millis() - this->start;
        this->start = millis();
        this->cycle++;

        return true;
    }

    /**
     * Get the delta time(in seconds) between this cycle and
     * the last
     */
    f32 dt() const {
        return ((float)this->delta) / 1000;
    }
    /**
     * Get the frames per second(naive implementation, no running average)
     */
    f32 fps() const {
        return 1 / max(this->dt(), 0.0001f);
    }
};