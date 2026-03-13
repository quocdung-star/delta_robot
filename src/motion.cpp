#include <Arduino.h>
#include "motion.h"
#include "axis.h"
#include "stepper.h"

static void step_axis(Axis &axis) {
    if (axis.stepsRemaining <= 0) {
        axis.moving = false;
        return;
    }

    stepper_step_axis(axis);
    axis.stepsRemaining--;
}

void motion_update(RobotState &state) {
    (void) state;
    // Reserved for higher-level motion planning; not used in this simple version
}

void motion_task(void *pv) {
    (void) pv;

    while (true) {
        if (axisX.moving) step_axis(axisX);
        if (axisY.moving) step_axis(axisY);
        if (axisZ.moving) step_axis(axisZ);

        vTaskDelay(1);
    }
}
