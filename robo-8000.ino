// src/tobo-8000.ino
#include <Arduino.h>

#include "src/control/control.h"
#include "src/input/input.h"
#include "src/system/loop.h"
#include "src/system/serial_commands.h"
#include "src/utils/log.h"
#include "src/utils/loop_profiler.h"
#include "src/webClient/network.h"

void setup() {
    Serial.begin(115200);
    log(INFO, "System starting");

    initControl();
    initLoop();
    initSerialCommands();
    initWebClient();
}

void loop() {
    beginLoopProfile();

    updateLoop();
    updateSerialCommands();

    endLoopProfile();
}