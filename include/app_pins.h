#pragma once

#include <Arduino.h>

// Pin definitions for the delta robot using 74HC595

// Shift register 74HC595 pins (ESP32)
#define DATA_PIN   21
#define CLOCK_PIN  16
#define LATCH_PIN  17

// Limit switches
#define LIMIT_X    36
#define LIMIT_Y    35
#define LIMIT_Z    34

// Endstop logic level when triggered
#define ENDSTOP_TRIGGERED LOW
// =====================================================
// 74HC595 output bit mapping
// Sửa lại theo đúng wiring thực tế của bạn
// Q0 -> bit 0, Q1 -> bit 1, ...
// =====================================================
#define SR_X_STEP  0
#define SR_X_DIR   1
#define SR_X_EN    2
