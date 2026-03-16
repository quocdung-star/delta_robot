#pragma once

void limit_switch_init();
bool limit_switch_is_triggered(int pin);
bool limit_x_triggered();
bool limit_y_triggered();
bool limit_z_triggered();
