#include <Arduino.h>
#include "limit_switch_monitor.h"
#include "limit_switch.h"

void limit_switch_monitor_init() {
    limit_switch_init();
}

void limit_switch_monitor_update() {
    // This module is intentionally silent during normal operation.
}
