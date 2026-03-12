#pragma once

#include "robot_state.h"

void delta_kinematics_forward(const RobotState &state);
void delta_kinematics_inverse(float x, float y, float z, RobotState &out);
