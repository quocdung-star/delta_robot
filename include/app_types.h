#pragma once

// Common type definitions

typedef unsigned long timestamp_t;

// Axis state used by motion control
struct Axis
{
    bool dir;
    volatile bool step;
    volatile bool moving;
    volatile long stepsRemaining;
};

extern Axis axisX;
extern Axis axisY;
extern Axis axisZ;
