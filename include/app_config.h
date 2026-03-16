#pragma once

// Motion configuration settings

// Basic motion timing (can be tuned later)
#define STEP_DELAY_US 200

// Step conversion used for simple serial angle tests after homing.
// Current hardware setting: 6400 pulses = 360 degrees.
#define STEPS_PER_REVOLUTION 6400.0f
#define STEPS_PER_DEGREE     (STEPS_PER_REVOLUTION / 360.0f)

// Homing safety limits (avoid endless motion if switch fails)
#define HOMING_TIMEOUT_MS 15000UL
#define HOMING_MAX_STEPS  200000L
