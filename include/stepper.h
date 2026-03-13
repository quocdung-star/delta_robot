#pragma once

#include "app_types.h"

// Stepper motor driver interface

void stepper_init();
void stepper_step_axis(Axis &axis);
