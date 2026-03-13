#pragma once

#include "robot_state.h"

void motion_update(RobotState &state);

// FreeRTOS motion task (ESP32)
void motion_task(void *pv);
