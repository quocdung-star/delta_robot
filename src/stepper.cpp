#include <Arduino.h>
#include <stdlib.h>
#include "emergency_stop.h"
#include "stepper.h"
#include "app_config.h"
#include "shift595.h"
#include "speed_control.h"

void stepper_init() {
    speed_control_init();
    shift595_init();
}

void stepper_step_axis(Axis &axis) {
    axis.step = true;
    shift595_write(shift595_build_output());
    delayMicroseconds(speed_control_get_step_delay_us());

    axis.step = false;
    shift595_write(shift595_build_output());
    delayMicroseconds(speed_control_get_step_delay_us());
}

void stepper_step_active_axes() {
    axisX.step = axisX.moving;
    axisY.step = axisY.moving;
    axisZ.step = axisZ.moving;
    shift595_write(shift595_build_output());
    delayMicroseconds(speed_control_get_step_delay_us());

    axisX.step = false;
    axisY.step = false;
    axisZ.step = false;
    shift595_write(shift595_build_output());
    delayMicroseconds(speed_control_get_step_delay_us());
}

bool stepper_move_axis(Axis &axis, bool dir, long steps, LimitSwitchReadFn homeSwitchTriggered) {
    if (steps <= 0) {
        axis.moving = false;
        axis.stepsRemaining = 0;
        return true;
    }

    if (!dir && homeSwitchTriggered != nullptr && homeSwitchTriggered()) {
        axis.moving = false;
        axis.stepsRemaining = 0;
        return false;
    }

    axis.dir = dir;
    axis.moving = true;
    axis.stepsRemaining = steps;

    while (axis.stepsRemaining > 0) {
        emergency_stop_poll();

        if (emergency_stop_is_active()) {
            axis.moving = false;
            axis.stepsRemaining = 0;
            return false;
        }

        if (!dir && homeSwitchTriggered != nullptr && homeSwitchTriggered()) {
            axis.moving = false;
            axis.stepsRemaining = 0;
            return false;
        }

        stepper_step_axis(axis);
        axis.stepsRemaining--;
    }

    axis.moving = false;
    axis.stepsRemaining = 0;
    return true;
}

static bool axis_can_move_toward_home(bool dir, LimitSwitchReadFn homeSwitchTriggered) {
    return dir || homeSwitchTriggered == nullptr || !homeSwitchTriggered();
}

bool stepper_move_axes(bool dirX, long stepsX, LimitSwitchReadFn homeX,
                       bool dirY, long stepsY, LimitSwitchReadFn homeY,
                       bool dirZ, long stepsZ, LimitSwitchReadFn homeZ) {
    const long totalX = stepsX > 0 ? stepsX : 0;
    const long totalY = stepsY > 0 ? stepsY : 0;
    const long totalZ = stepsZ > 0 ? stepsZ : 0;

    axisX.dir = dirX;
    axisY.dir = dirY;
    axisZ.dir = dirZ;

    axisX.stepsRemaining = totalX;
    axisY.stepsRemaining = totalY;
    axisZ.stepsRemaining = totalZ;

    axisX.moving = totalX > 0;
    axisY.moving = totalY > 0;
    axisZ.moving = totalZ > 0;

    if (!axis_can_move_toward_home(dirX, homeX)) {
        axisX.moving = false;
        axisX.stepsRemaining = 0;
    }
    if (!axis_can_move_toward_home(dirY, homeY)) {
        axisY.moving = false;
        axisY.stepsRemaining = 0;
    }
    if (!axis_can_move_toward_home(dirZ, homeZ)) {
        axisZ.moving = false;
        axisZ.stepsRemaining = 0;
    }

    const long maxSteps = max(totalX, max(totalY, totalZ));
    if (maxSteps <= 0) {
        return true;
    }

    long accX = 0;
    long accY = 0;
    long accZ = 0;
    bool aborted = false;

    for (long i = 0; i < maxSteps; ++i) {
        emergency_stop_poll();
        if (emergency_stop_is_active()) {
            aborted = true;
            break;
        }

        if (axisX.moving && !axis_can_move_toward_home(dirX, homeX)) {
            aborted = true;
            break;
        }
        if (axisY.moving && !axis_can_move_toward_home(dirY, homeY)) {
            aborted = true;
            break;
        }
        if (axisZ.moving && !axis_can_move_toward_home(dirZ, homeZ)) {
            aborted = true;
            break;
        }

        axisX.step = false;
        axisY.step = false;
        axisZ.step = false;

        accX += totalX;
        if (axisX.moving && axisX.stepsRemaining > 0 && accX >= maxSteps) {
            axisX.step = true;
            axisX.stepsRemaining--;
            accX -= maxSteps;
            if (axisX.stepsRemaining <= 0) {
                axisX.moving = false;
            }
        }

        accY += totalY;
        if (axisY.moving && axisY.stepsRemaining > 0 && accY >= maxSteps) {
            axisY.step = true;
            axisY.stepsRemaining--;
            accY -= maxSteps;
            if (axisY.stepsRemaining <= 0) {
                axisY.moving = false;
            }
        }

        accZ += totalZ;
        if (axisZ.moving && axisZ.stepsRemaining > 0 && accZ >= maxSteps) {
            axisZ.step = true;
            axisZ.stepsRemaining--;
            accZ -= maxSteps;
            if (axisZ.stepsRemaining <= 0) {
                axisZ.moving = false;
            }
        }

        if (!axisX.step && !axisY.step && !axisZ.step) {
            continue;
        }

        shift595_write(shift595_build_output());
        delayMicroseconds(speed_control_get_step_delay_us());

        axisX.step = false;
        axisY.step = false;
        axisZ.step = false;
        shift595_write(shift595_build_output());
        delayMicroseconds(speed_control_get_step_delay_us());
    }

    axisX.step = false;
    axisY.step = false;
    axisZ.step = false;
    axisX.moving = false;
    axisY.moving = false;
    axisZ.moving = false;

    const bool completed = !aborted
        && axisX.stepsRemaining == 0
        && axisY.stepsRemaining == 0
        && axisZ.stepsRemaining == 0;

    if (!completed) {
        axisX.stepsRemaining = 0;
        axisY.stepsRemaining = 0;
        axisZ.stepsRemaining = 0;
    }

    return completed;
}
