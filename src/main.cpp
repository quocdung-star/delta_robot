#include <Arduino.h>
#include "app_pins.h"

static uint8_t srState = 0x00;

// Thời gian giữa 2 xung step
static const unsigned long STEP_INTERVAL_US = 800;

void writeShiftRegister(uint8_t value) {
    digitalWrite(LATCH_PIN, LOW);
    shiftOut(DATA_PIN, CLOCK_PIN, MSBFIRST, value);
    digitalWrite(LATCH_PIN, HIGH);
}

void setShiftBit(uint8_t bit, bool state) {
    if (state) {
        srState |= (1 << bit);
    } else {
        srState &= ~(1 << bit);
    }
    writeShiftRegister(srState);
}

void pulseStepX() {
    setShiftBit(SR_X_STEP, HIGH);
    delayMicroseconds(5);
    setShiftBit(SR_X_STEP, LOW);
}

bool isLimitXTriggered() {
    return digitalRead(LIMIT_X) == ENDSTOP_TRIGGERED;
}

void startMotorX() {
    // Enable driver: thường LOW = enable
    setShiftBit(SR_X_EN, LOW);

    // Quay theo chiều HIGH
    setShiftBit(SR_X_DIR, HIGH);
}

void stopMotorX() {
    // Disable driver: thường HIGH = disable
    setShiftBit(SR_X_EN, HIGH);
}

void setup() {
    Serial.begin(115200);

    pinMode(DATA_PIN, OUTPUT);
    pinMode(CLOCK_PIN, OUTPUT);
    pinMode(LATCH_PIN, OUTPUT);

    pinMode(LIMIT_X, INPUT);
    pinMode(LIMIT_Y, INPUT);
    pinMode(LIMIT_Z, INPUT);

    writeShiftRegister(srState);

    Serial.println("Motor X quay theo chieu HIGH den khi LIMIT_X duoc kich hoat");

    if (isLimitXTriggered()) {
        Serial.println("LIMIT_X dang kich hoat san -> khong cho motor chay");
        stopMotorX();
    } else {
        startMotorX();
    }
}

void loop() {
    static unsigned long lastStepUs = 0;
    static bool stoppedMessagePrinted = false;

    if (isLimitXTriggered()) {
        stopMotorX();

        if (!stoppedMessagePrinted) {
            Serial.println("LIMIT_X da duoc kich hoat -> dung motor X");
            stoppedMessagePrinted = true;
        }
        return;
    }

    unsigned long nowUs = micros();
    if (nowUs - lastStepUs >= STEP_INTERVAL_US) {
        lastStepUs = nowUs;
        pulseStepX();
    }
}