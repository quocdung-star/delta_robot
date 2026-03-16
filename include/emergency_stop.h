#pragma once

void emergency_stop_init();
void emergency_stop_request();
void emergency_stop_clear();
void emergency_stop_poll();
bool emergency_stop_is_active();
