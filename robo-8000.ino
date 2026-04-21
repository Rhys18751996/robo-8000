#include <Arduino.h>

#include "src/control/control.h"
#include "src/input/input.h"
#include "src/system/loop.h"
#include "src/system/serial_commands.h"
#include "src/utils/log.h"

void setup() {
    Serial.begin(115200);
    log(INFO, "System starting");

    initControl();
    initLoop();
    initSerialCommands();
}

void loop() {
    updateLoop();
    updateSerialCommands();
}