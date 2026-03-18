#include <Arduino.h>
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
