#include <Arduino.h>
#include "homing.h"
#include "app_config.h"
#include "stepper.h"

bool home_axis(Axis &axis, int limitPin) {
    axis.dir = true; // homing direction, adjust if needed

    // If already triggered, consider homed (or wiring inverted)
    if (digitalRead(limitPin) == ENDSTOP_TRIGGERED) {
        axis.moving = false;
        axis.stepsRemaining = 0;
        return true;
    }

    const unsigned long startMs = millis();
    long steps = 0;

    while (digitalRead(limitPin) != ENDSTOP_TRIGGERED) {
        stepper_step_axis(axis);
        steps++;

        if ((millis() - startMs) > HOMING_TIMEOUT_MS) {
            axis.moving = false;
            axis.stepsRemaining = 0;
            return false;
        }

        if (steps > HOMING_MAX_STEPS) {
            axis.moving = false;
            axis.stepsRemaining = 0;
            return false;
        }
    }

    axis.moving = false;
    axis.stepsRemaining = 0;
    return true;
}

void homing_run() {
    // Example: home all axes in sequence (can be expanded)
    home_axis(axisX, LIMIT_X);
    home_axis(axisY, LIMIT_Y);
    home_axis(axisZ, LIMIT_Z);
}
