#pragma once

#include <Arduino.h>
#include "app_pins.h"
#include "app_types.h"

// Shift register (74HC595) interface

void shift595_init();
void shift595_write(uint8_t value);
uint8_t shift595_build_output();
