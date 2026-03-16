#include <Arduino.h>
#include <math.h>
#include <stdio.h>
#include "app_pins.h"
#include "app_config.h"
#include "axis.h"
#include "emergency_stop.h"
#include "homing.h"
#include "limit_switch_monitor.h"
#include "speed_control.h"
#include "stepper.h"

static bool homingCompleted = false;
static String serialBuffer;

static void print_homing_status(const HomingReport &report) {
    Serial.print("HOME_X: ");
    Serial.println(report.xHomed ? "OK" : "FAIL");
    Serial.print("HOME_Y: ");
    Serial.println(report.yHomed ? "OK" : "FAIL");
    Serial.print("HOME_Z: ");
    Serial.println(report.zHomed ? "OK" : "FAIL");
    Serial.print("HOMING_ALL: ");
    Serial.println(report.allHomed ? "OK" : "FAIL");
}

static void print_input_prompt() {
    Serial.println("Enter angles as: X Y Z");
    Serial.println("Positive angle -> HIGH direction, negative angle -> LOW direction");
    Serial.println("Use $V <delay_us> to change speed, use $S for emergency stop");
}

static long angle_to_steps(float angleDeg) {
    const float steps = fabsf(angleDeg) * STEPS_PER_DEGREE;
    return (long)(steps + 0.5f);
}

static bool move_axis_from_angle(const char *axisName, Axis &axis, float angleDeg) {
    const bool dirHigh = angleDeg >= 0.0f;
    const long steps = angle_to_steps(angleDeg);

    Serial.print(axisName);
    Serial.print(": angle=");
    Serial.print(angleDeg, 3);
    Serial.print(" deg, dir=");
    Serial.print(dirHigh ? "HIGH" : "LOW");
    Serial.print(", steps=");
    Serial.println(steps);

    if (steps == 0) {
        return true;
    }

    return stepper_move_axis(axis, dirHigh, steps);
}

static void handle_angle_command(const String &command) {
    float angleX = 0.0f;
    float angleY = 0.0f;
    float angleZ = 0.0f;
    char buffer[96];

    command.toCharArray(buffer, sizeof(buffer));

    if (sscanf(buffer, "%f %f %f", &angleX, &angleY, &angleZ) != 3) {
        Serial.println("Invalid input. Use format: X Y Z");
        print_input_prompt();
        return;
    }

    if (emergency_stop_is_active()) {
        Serial.println("Emergency stop is active. Reboot or clear stop state before moving.");
        return;
    }

    const bool xOk = move_axis_from_angle("X", axisX, angleX);
    const bool yOk = xOk ? move_axis_from_angle("Y", axisY, angleY) : false;
    const bool zOk = yOk ? move_axis_from_angle("Z", axisZ, angleZ) : false;

    if (xOk && yOk && zOk) {
        Serial.println("MOVE_DONE");
    } else {
        Serial.println("MOVE_ABORTED");
    }

    print_input_prompt();
}

static void handle_speed_command(const String &command) {
    unsigned long delayUs = 0;

    if (sscanf(command.c_str(), "$V %lu", &delayUs) != 1) {
        Serial.println("Invalid speed command. Use: $V <delay_us>");
        return;
    }

    speed_control_set_step_delay_us(delayUs);
    Serial.print("STEP_DELAY_US set to ");
    Serial.println(speed_control_get_step_delay_us());
}

static void handle_serial_command(const String &command) {
    if (command == "$S") {
        emergency_stop_request();
        Serial.println("EMERGENCY_STOP_ACTIVE");
        return;
    }

    if (command.startsWith("$V ")) {
        handle_speed_command(command);
        print_input_prompt();
        return;
    }

    handle_angle_command(command);
}

void setup() {
    Serial.begin(115200);

    pinMode(DATA_PIN, OUTPUT);
    pinMode(CLOCK_PIN, OUTPUT);
    pinMode(LATCH_PIN, OUTPUT);

    limit_switch_monitor_init();
    emergency_stop_init();
    axis_init();
    stepper_init();

    Serial.println("Starting homing test...");
    const HomingReport report = homing_run();
    print_homing_status(report);
    homingCompleted = report.allHomed;

    if (homingCompleted) {
        Serial.println("Homing complete. Angle input unlocked.");
        print_input_prompt();
    } else {
        Serial.println("Homing failed. Angle input locked.");
    }
}

void loop() {
    limit_switch_monitor_update();

    if (!homingCompleted) {
        return;
    }

    while (Serial.available()) {
        const char c = (char) Serial.read();

        if (c == '\n' || c == '\r') {
            serialBuffer.trim();

            if (serialBuffer.length() > 0) {
                handle_serial_command(serialBuffer);
                serialBuffer = "";
            }
        } else {
            serialBuffer += c;
        }
    }
}
