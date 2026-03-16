#include <Arduino.h>
#include "emergency_stop.h"
#include "homing.h"
#include "app_config.h"
#include "limit_switch.h"
#include "stepper.h"

bool home_axis(Axis &axis, LimitSwitchReadFn isTriggered) {
    axis.dir = true; // homing direction, adjust if needed

    // If already triggered, consider homed (or wiring inverted)
    if (isTriggered()) {
        axis.moving = false;
        axis.stepsRemaining = 0;
        return true;
    }

    const unsigned long startMs = millis();
    long steps = 0;

    while (!isTriggered()) {
        emergency_stop_poll();

        if (emergency_stop_is_active()) {
            axis.moving = false;
            axis.stepsRemaining = 0;
            return false;
        }

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

HomingReport homing_run() {
    HomingReport report = {};

    report.xHomed = home_axis(axisX, limit_x_triggered);
    report.yHomed = home_axis(axisY, limit_y_triggered);
    report.zHomed = home_axis(axisZ, limit_z_triggered);
    report.allHomed = report.xHomed && report.yHomed && report.zHomed;

    return report;
}
