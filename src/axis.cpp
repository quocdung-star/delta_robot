#include "axis.h"

Axis axisX = { false, false, false, 0 };
Axis axisY = { false, false, false, 0 };
Axis axisZ = { false, false, false, 0 };

void axis_init() {
    axisX.dir = false;
    axisX.step = false;
    axisX.moving = false;
    axisX.stepsRemaining = 0;

    axisY.dir = false;
    axisY.step = false;
    axisY.moving = false;
    axisY.stepsRemaining = 0;

    axisZ.dir = false;
    axisZ.step = false;
    axisZ.moving = false;
    axisZ.stepsRemaining = 0;
}
