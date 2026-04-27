// src/tobo-8000.ino
#include <Arduino.h>

#include "src/control/control.h"
#include "src/input/input.h"
#include "src/system/loop.h"
#include "src/system/serial_commands.h"
#include "src/utils/log.h"
#include "src/utils/loop_profiler.h"
#include "src/webClient/network.h"
#include "src/config/preferences_storage.h"
#include "src/motor/motor.h"

void setup() {
    Serial.begin(115200);

    esp_reset_reason_t reason = esp_reset_reason();
    logf(INFO, "Reset reason: %d", reason);
    
    log(INFO, "System starting");

    initPreferencesStorage(); 

    initControl();
    initLoop();
    initSerialCommands();
    initMotors();
    initWebClient();
}

void loop() {
    //beginLoopProfile();

    updateLoop();
    updateSerialCommands();

    //endLoopProfile();
}