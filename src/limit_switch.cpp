#include <Arduino.h>
#include "app_pins.h"
#include "limit_switch.h"

void limit_switch_init() {
    pinMode(LIMIT_X, INPUT);
    pinMode(LIMIT_Y, INPUT);
    pinMode(LIMIT_Z, INPUT);
}

bool limit_switch_is_triggered(int pin) {
    return digitalRead(pin) == ENDSTOP_TRIGGERED;
}

bool limit_x_triggered() {
    return limit_switch_is_triggered(LIMIT_X);
}

bool limit_y_triggered() {
    return limit_switch_is_triggered(LIMIT_Y);
}

bool limit_z_triggered() {
    return limit_switch_is_triggered(LIMIT_Z);
}
