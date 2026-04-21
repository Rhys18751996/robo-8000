// robo-8000.ino

#include <Arduino.h>

#include "src/control/control.h"
#include "src/system/loop.h"
#include "src/utils/log.h"

void setup() {
    Serial.begin(115200);
    log(INFO, "System starting");
    initControl();
    initLoop();
}

void loop() {
    updateLoop();
}
