#include "shift595.h"
#include "axis.h"

void shift595_init() {
    // No special init for 74HC595 here; pins are configured in setup
    shift595_write(0);
}

void shift595_write(uint8_t value) {
    digitalWrite(LATCH_PIN, LOW);
    shiftOut(DATA_PIN, CLOCK_PIN, MSBFIRST, value);
    digitalWrite(LATCH_PIN, HIGH);
}

uint8_t shift595_build_output() {
    uint8_t data = 0;

    // bit0 ENA always LOW (enable always on)
    if (axisX.step) data |= (1 << 1);
    if (axisX.dir)  data |= (1 << 2);

    if (axisZ.step) data |= (1 << 3);
    if (axisZ.dir)  data |= (1 << 4);

    if (axisY.step) data |= (1 << 5);
    if (axisY.dir)  data |= (1 << 6);

    return data;
}
