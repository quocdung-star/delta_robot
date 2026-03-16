#pragma once

#include "app_types.h"

// Stepper motor driver interface

void stepper_init();
void stepper_step_axis(Axis &axis);
bool stepper_move_axis(Axis &axis, bool dir, long steps);
