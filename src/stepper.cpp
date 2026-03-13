#include <Arduino.h>
#include "stepper.h"
#include "app_config.h"
#include "shift595.h"

void stepper_init() {
    shift595_init();
}

void stepper_step_axis(Axis &axis) {
    axis.step = true;
    shift595_write(shift595_build_output());
    delayMicroseconds(STEP_DELAY_US);

    axis.step = false;
    shift595_write(shift595_build_output());
    delayMicroseconds(STEP_DELAY_US);
}
