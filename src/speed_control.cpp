#include "app_config.h"
#include "speed_control.h"

static unsigned long currentStepDelayUs = STEP_DELAY_US;

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
