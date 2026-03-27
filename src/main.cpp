#include <Arduino.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include "app_pins.h"
#include "axis.h"
#include "emergency_stop.h"
#include "homing.h"
#include "limit_switch.h"
#include "speed_control.h"
#include "stepper.h"

static bool homingCompleted = false;
static String serialBuffer;
enum class MotionInputAxis : uint8_t {
    X = 0,
    Y,
    Z,
};

static MotionInputAxis nextMotionInputAxis = MotionInputAxis::X;
static long pendingXPulses = 0;
static long pendingYPulses = 0;
static long pendingZPulses = 0;

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
    Serial.println("Enter pulses one axis at a time, press Enter after each value.");
    Serial.println("Input order: X, then Y, then Z.");
    Serial.println("Positive pulses -> HIGH direction, negative pulses -> LOW direction");
    Serial.println("Use $H to start homing, $V <rpm> to change speed, use $S for emergency stop");
    Serial.println("Enter X pulses:");
}

static void print_waiting_home_prompt() {
    Serial.println("Waiting for homing command.");
    Serial.println("Use $H to start homing, $V <rpm> to change speed, use $S for emergency stop");
}

static void run_homing_command() {
    if (emergency_stop_is_active()) {
        Serial.println("Emergency stop is active. Clear stop state before homing.");
        return;
    }

    Serial.println("Starting homing...");
    const HomingReport report = homing_run();
    print_homing_status(report);
    homingCompleted = report.allHomed;

    if (homingCompleted) {
        Serial.println("Homing complete. Angle input unlocked.");
        print_input_prompt();
    } else {
        Serial.println("Homing failed. Angle input locked.");
        print_waiting_home_prompt();
    }
}

static void reset_motion_input() {
    nextMotionInputAxis = MotionInputAxis::X;
    pendingXPulses = 0;
    pendingYPulses = 0;
    pendingZPulses = 0;
}

static const char *axis_name(MotionInputAxis axis) {
    switch (axis) {
        case MotionInputAxis::X:
            return "X";
        case MotionInputAxis::Y:
            return "Y";
        case MotionInputAxis::Z:
            return "Z";
        default:
            return "?";
    }
}

static void print_next_axis_prompt() {
    Serial.print("Enter ");
    Serial.print(axis_name(nextMotionInputAxis));
    Serial.println(" pulses:");
}

static bool parse_pulses(const String &command, long &pulses) {
    char *endPtr = nullptr;
    pulses = strtol(command.c_str(), &endPtr, 10);

    if (endPtr == command.c_str()) {
        return false;
    }

    while (*endPtr != '\0') {
        if (!isspace((unsigned char) *endPtr)) {
            return false;
        }
        endPtr++;
    }

    return true;
}

static void print_move_request(const char *axisName, long pulses) {
    const bool dirHigh = pulses >= 0;
    const long steps = labs(pulses);

    Serial.print(axisName);
    Serial.print(": pulses=");
    Serial.print(pulses);
    Serial.print(", dir=");
    Serial.print(dirHigh ? "HIGH" : "LOW");
    Serial.print(", steps=");
    Serial.println(steps);
}

static void handle_pulse_command(const String &command) {
    long pulses = 0;

    if (!parse_pulses(command, pulses)) {
        Serial.print("Invalid pulse input for axis ");
        Serial.print(axis_name(nextMotionInputAxis));
        Serial.println(". Enter an integer value.");
        print_next_axis_prompt();
        return;
    }

    if (emergency_stop_is_active()) {
        Serial.println("Emergency stop is active. Reboot or clear stop state before moving.");
        return;
    }

    switch (nextMotionInputAxis) {
        case MotionInputAxis::X:
            pendingXPulses = pulses;
            nextMotionInputAxis = MotionInputAxis::Y;
            print_next_axis_prompt();
            return;
        case MotionInputAxis::Y:
            pendingYPulses = pulses;
            nextMotionInputAxis = MotionInputAxis::Z;
            print_next_axis_prompt();
            return;
        case MotionInputAxis::Z:
            pendingZPulses = pulses;
            break;
    }

    print_move_request("X", pendingXPulses);
    print_move_request("Y", pendingYPulses);
    print_move_request("Z", pendingZPulses);

    const bool moveOk = stepper_move_axes(
        pendingXPulses >= 0, labs(pendingXPulses), limit_x_triggered,
        pendingYPulses >= 0, labs(pendingYPulses), limit_y_triggered,
        pendingZPulses >= 0, labs(pendingZPulses), limit_z_triggered
    );

    if (moveOk) {
        Serial.println("MOVE_DONE");
    } else {
        Serial.println("MOVE_ABORTED");
    }

    reset_motion_input();
    print_input_prompt();
}

static void handle_speed_command(const String &command) {
    float rpm = 0.0f;

    if (sscanf(command.c_str(), "$V %f", &rpm) != 1 || rpm <= 0.0f) {
        Serial.println("Invalid speed command. Use: $V <rpm>");
        return;
    }

    speed_control_set_rpm(rpm);
    Serial.print("RPM set to ");
    Serial.println(speed_control_get_rpm(), 3);
    Serial.print("STEP_DELAY_US = ");
    Serial.println(speed_control_get_step_delay_us());
}

static void handle_serial_command(const String &command) {
    if (command == "$S") {
        emergency_stop_request();
        Serial.println("EMERGENCY_STOP_ACTIVE");
        return;
    }

    if (command == "$H") {
        reset_motion_input();
        run_homing_command();
        return;
    }

    if (command.startsWith("$V ")) {
        handle_speed_command(command);
        if (homingCompleted) {
            print_input_prompt();
        } else {
            print_waiting_home_prompt();
        }
        return;
    }

    if (!homingCompleted) {
        Serial.println("Homing has not started. Use $H first.");
        print_waiting_home_prompt();
        return;
    }

    handle_pulse_command(command);
}

void setup() {
    Serial.begin(115200);

    pinMode(DATA_PIN, OUTPUT);
    pinMode(CLOCK_PIN, OUTPUT);
    pinMode(LATCH_PIN, OUTPUT);

    limit_switch_init();
    emergency_stop_init();
    axis_init();
    stepper_init();
    reset_motion_input();

    Serial.println("System ready. Homing is manual.");
    print_waiting_home_prompt();
}

void loop() {
    limit_switch_update();

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
