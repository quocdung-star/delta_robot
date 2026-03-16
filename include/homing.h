#pragma once

#include "axis.h"

typedef bool (*LimitSwitchReadFn)();

struct HomingReport {
    bool xHomed;
    bool yHomed;
    bool zHomed;
    bool allHomed;
};

// Simple homing for a single axis using a limit switch reader callback.
bool home_axis(Axis &axis, LimitSwitchReadFn isTriggered);

// High-level homing routine with per-axis result reporting.
HomingReport homing_run();
