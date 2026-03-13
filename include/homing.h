#pragma once

#include "axis.h"
#include "app_pins.h"

// Simple homing for a single axis
bool home_axis(Axis &axis, int limitPin);

// High-level homing routine (can be expanded later)
void homing_run();
