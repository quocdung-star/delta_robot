#include <Arduino.h>
#include "protocol.h"
#include "axis.h"
#include "homing.h"
#include "motion.h"
#include "app_pins.h"
#include "stepper.h"

static void parse_command(String cmd);
static void serial_task(void *pv);

// Helper functions to read limit switches directly using app_pins.h
static bool limit_x_triggered() {
    return digitalRead(LIMIT_X) == ENDSTOP_TRIGGERED;
}

static bool limit_y_triggered() {
    return digitalRead(LIMIT_Y) == ENDSTOP_TRIGGERED;
}

static bool limit_z_triggered() {
    return digitalRead(LIMIT_Z) == ENDSTOP_TRIGGERED;
}

void protocol_init() {
    Serial.begin(115200);

    // Pin configuration
    pinMode(DATA_PIN, OUTPUT);
    pinMode(CLOCK_PIN, OUTPUT);
    pinMode(LATCH_PIN, OUTPUT);

    pinMode(LIMIT_X, INPUT);
    pinMode(LIMIT_Y, INPUT);
    pinMode(LIMIT_Z, INPUT);

    axis_init();
    stepper_init();

    // Create motion task
    xTaskCreatePinnedToCore(
        motion_task,
        "motion",
        4096,
        nullptr,
        2,
        nullptr,
        1
    );

    // Create serial task
    xTaskCreatePinnedToCore(
        serial_task,
        "serial",
        4096,
        nullptr,
        1,
        nullptr,
        0
    );

    Serial.println("System ready");
}

void protocol_process() {
    // All work is done in FreeRTOS tasks for this simple example
}

static void parse_command(String cmd) {
    cmd.trim();
    cmd.toUpperCase();

    if (cmd == "$HX") {
        Serial.println("Homing X");
        bool ok = home_axis(axisX, LIMIT_X);
        Serial.println(ok ? "HX_DONE" : "HX_FAIL");
    } else if (cmd == "$HY") {
        Serial.println("Homing Y");
        bool ok = home_axis(axisY, LIMIT_Y);
        Serial.println(ok ? "HY_DONE" : "HY_FAIL");
    } else if (cmd == "$HZ") {
        Serial.println("Homing Z");
        bool ok = home_axis(axisZ, LIMIT_Z);
        Serial.println(ok ? "HZ_DONE" : "HZ_FAIL");
    } else if (cmd == "$LX") {
        Serial.print("LIMIT_X: ");
        Serial.println(limit_x_triggered() ? "TRIGGERED" : "OPEN");
    } else if (cmd == "$LY") {
        Serial.print("LIMIT_Y: ");
        Serial.println(limit_y_triggered() ? "TRIGGERED" : "OPEN");
    } else if (cmd == "$LZ") {
        Serial.print("LIMIT_Z: ");
        Serial.println(limit_z_triggered() ? "TRIGGERED" : "OPEN");
    } else if (cmd == "$LS") {
        Serial.print("LIMIT_X: ");
        Serial.println(limit_x_triggered() ? "TRIGGERED" : "OPEN");
        Serial.print("LIMIT_Y: ");
        Serial.println(limit_y_triggered() ? "TRIGGERED" : "OPEN");
        Serial.print("LIMIT_Z: ");
        Serial.println(limit_z_triggered() ? "TRIGGERED" : "OPEN");
    } else if (cmd.startsWith("MX ")) {
        long s = cmd.substring(3).toInt();
        axisX.dir = true;
        axisX.stepsRemaining = s;
        axisX.moving = true;
    } else if (cmd.startsWith("MY ")) {
        long s = cmd.substring(3).toInt();
        axisY.dir = true;
        axisY.stepsRemaining = s;
        axisY.moving = true;
    } else if (cmd.startsWith("MZ ")) {
        long s = cmd.substring(3).toInt();
        axisZ.dir = true;
        axisZ.stepsRemaining = s;
        axisZ.moving = true;
    } else {
        Serial.print("UNKNOWN_CMD: ");
        Serial.println(cmd);
    }
}

static void serial_task(void *pv) {
    (void) pv;

    String cmd = "";

    while (true) {
        while (Serial.available()) {
            char c = Serial.read();

            if (c == '\n' || c == '\r') {
                if (cmd.length() > 0) {
                    parse_command(cmd);
                    cmd = "";
                }
            } else {
                cmd += c;
            }
        }

        vTaskDelay(10);
    }
}
