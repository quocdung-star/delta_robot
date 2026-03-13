#pragma once

// Motion configuration settings

// Basic motion timing (can be tuned later)
#define STEP_DELAY_US 200

// Homing safety limits (avoid endless motion if switch fails)
#define HOMING_TIMEOUT_MS 15000UL
#define HOMING_MAX_STEPS  200000L
