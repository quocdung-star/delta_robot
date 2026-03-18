#pragma once

void speed_control_init();
void speed_control_set_step_delay_us(unsigned long delayUs);
unsigned long speed_control_get_step_delay_us();
void speed_control_set_rpm(float rpm);
float speed_control_get_rpm();
