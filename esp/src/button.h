/**
 * [btn.h] Button with self-contained state
 */
#pragma once

#include <stdint.h>
#include <Arduino.h>

#include "num.h"

/**
 * An active-low button
 */
class Button {
    u8 pin;             // IO Pin to read from
    u64 time;           // Last state transition time(ms)
    enum {
        Pressed,        // Button has reached a debounced down state
        Released,       // Button has reached a debounced up state
        Pressing,       // Button is maybe being pressed
        Releasing,      // Button is maybe being released
    } state;
public:
    /**
     * Milliseconds during which a button must held at a certain state
     * for that state to be acknolwedged.
     */
    static const u64 DEBOUNCE = 10;

    /**
     * Initialize a pull-up button at the given IO pin
     */
    Button(u8 pin) : pin(pin), time(millis()), state(Released) {
        pinMode(pin, INPUT_PULLUP);
    }

    /**
     * Read's the physical button's current state and updates
     * the internal state accordingly. Returns -1 if unpressed
     * just now, +1 if pressed just now, 0 if same as last poll.
     */
    i32 poll() {
        int input = digitalRead(this->pin);
        // FSM
        switch (this->state) {
            case Released:
                // Begin pressing
                if (!input) {
                    this->time = millis();
                    this->state = Pressing;
                }
                // Same as before
                return 0;
            case Pressing:
                // Cancel pressing
                if (input) {
                    this->state = Released;
                }
                // Debounced
                if (millis() - this->time >= DEBOUNCE) {
                    this->state = Pressed;
                    // Just pressed
                    return 1;
                }
                // Same as before
                return 0;
            case Pressed:
                // Begin release
                if (input) {
                    this->time = millis();
                    this->state = Releasing;
                }
                // Same as before
                return 0;
            case Releasing:
                // Cancel releasing
                if (!input) {
                    this->state = Pressed;
                }
                // Debounced
                if (millis() - this->time >= DEBOUNCE) {
                    this->state = Released;
                    // Just released
                    return -1;
                }
                // Same as before
                return 0;
        }
        return 0;
    }
};