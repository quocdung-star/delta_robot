#pragma once

#include "app_types.h"

// Stepper motor driver interface
typedef bool (*LimitSwitchReadFn)();

void stepper_init();
void stepper_step_axis(Axis &axis);
void stepper_step_active_axes();
bool stepper_move_axis(Axis &axis, bool dir, long steps, LimitSwitchReadFn homeSwitchTriggered = nullptr);
bool stepper_move_axes(bool dirX, long stepsX, LimitSwitchReadFn homeX,
                       bool dirY, long stepsY, LimitSwitchReadFn homeY,
                       bool dirZ, long stepsZ, LimitSwitchReadFn homeZ);
