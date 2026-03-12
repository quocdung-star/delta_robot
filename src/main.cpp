#include "app_config.h"
#include "protocol.h"

void setup() {
    protocol_init();
}

void loop() {
    protocol_process();
}
