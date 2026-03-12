#pragma once

// Shift register (74HC595) interface

void shift595_init();
void shift595_write(uint8_t value);
