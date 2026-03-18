#include <Arduino.h>
#include "emergency_stop.h"
#include "homing.h"
#include "app_config.h"
#include "limit_switch.h"
#include "stepper.h"

bool home_axis(Axis &axis, LimitSwitchReadFn isTriggered) {
    axis.dir = false; // homing direction set to LOW

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
    bool xFailed = false;
    bool yFailed = false;
    bool zFailed = false;
    long xSteps = 0;
    long ySteps = 0;
    long zSteps = 0;
    const unsigned long startMs = millis();

    axisX.dir = false;
    axisY.dir = false;
    axisZ.dir = false;

    report.xHomed = limit_x_triggered();
    report.yHomed = limit_y_triggered();
    report.zHomed = limit_z_triggered();

    axisX.moving = !report.xHomed;
    axisY.moving = !report.yHomed;
    axisZ.moving = !report.zHomed;

    while (axisX.moving || axisY.moving || axisZ.moving) {
        emergency_stop_poll();

        if (emergency_stop_is_active()) {
            break;
        }

        stepper_step_active_axes();

        if (axisX.moving) {
            xSteps++;
        }
        if (axisY.moving) {
            ySteps++;
        }
        if (axisZ.moving) {
            zSteps++;
        }

        report.xHomed = limit_x_triggered();
        report.yHomed = limit_y_triggered();
        report.zHomed = limit_z_triggered();

        if (report.xHomed) {
            axisX.moving = false;
        }
        if (report.yHomed) {
            axisY.moving = false;
        }
        if (report.zHomed) {
            axisZ.moving = false;
        }

        if ((millis() - startMs) > HOMING_TIMEOUT_MS) {
            xFailed = !report.xHomed;
            yFailed = !report.yHomed;
            zFailed = !report.zHomed;
            break;
        }

        if (xSteps > HOMING_MAX_STEPS) {
            xFailed = true;
            axisX.moving = false;
        }
        if (ySteps > HOMING_MAX_STEPS) {
            yFailed = true;
            axisY.moving = false;
        }
        if (zSteps > HOMING_MAX_STEPS) {
            zFailed = true;
            axisZ.moving = false;
        }
    }

    axisX.moving = false;
    axisY.moving = false;
    axisZ.moving = false;
    axisX.stepsRemaining = 0;
    axisY.stepsRemaining = 0;
    axisZ.stepsRemaining = 0;

    if (emergency_stop_is_active()) {
        report.xHomed = false;
        report.yHomed = false;
        report.zHomed = false;
    } else {
        report.xHomed = report.xHomed && !xFailed;
        report.yHomed = report.yHomed && !yFailed;
        report.zHomed = report.zHomed && !zFailed;
    }

    report.allHomed = report.xHomed && report.yHomed && report.zHomed;

    return report;
}
