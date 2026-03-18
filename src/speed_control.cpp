#include "app_config.h"
#include "speed_control.h"

static unsigned long currentStepDelayUs = STEP_DELAY_US;

static unsigned long rpm_to_step_delay_us(float rpm) {
    if (rpm <= 0.0f) {
        return STEP_DELAY_US;
    }

    const float pulsesPerMinute = rpm * STEPS_PER_REVOLUTION;
    const float stepDelayUs = 60000000.0f / (pulsesPerMinute * 2.0f);

    if (stepDelayUs < 1.0f) {
        return 1;
    }

    return (unsigned long)(stepDelayUs + 0.5f);
}

static float step_delay_us_to_rpm(unsigned long delayUs) {
    if (delayUs == 0) {
        delayUs = 1;
    }

    return 60000000.0f / (STEPS_PER_REVOLUTION * 2.0f * (float)delayUs);
}

void speed_control_init() {
    currentStepDelayUs = STEP_DELAY_US;
}

void speed_control_set_step_delay_us(unsigned long delayUs) {
    if (delayUs == 0) {
        currentStepDelayUs = 1;
        return;
    }

    currentStepDelayUs = delayUs;
}

unsigned long speed_control_get_step_delay_us() {
    return currentStepDelayUs;
}

void speed_control_set_rpm(float rpm) {
    currentStepDelayUs = rpm_to_step_delay_us(rpm);
}

float speed_control_get_rpm() {
    return step_delay_us_to_rpm(currentStepDelayUs);
}
