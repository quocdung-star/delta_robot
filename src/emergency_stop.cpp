#include <Arduino.h>
#include "emergency_stop.h"

static bool emergencyStopActive = false;
static String emergencyBuffer;

void emergency_stop_init() {
    emergencyStopActive = false;
    emergencyBuffer = "";
}

void emergency_stop_request() {
    emergencyStopActive = true;
}

void emergency_stop_clear() {
    emergencyStopActive = false;
    emergencyBuffer = "";
}

bool emergency_stop_is_active() {
    return emergencyStopActive;
}

void emergency_stop_poll() {
    while (Serial.available()) {
        const char c = (char) Serial.read();

        if (c == '\n' || c == '\r') {
            emergencyBuffer.trim();

            if (emergencyBuffer == "$S") {
                emergency_stop_request();
            }

            emergencyBuffer = "";
        } else {
            emergencyBuffer += c;
        }
    }
}
