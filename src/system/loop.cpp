// src/system/loop.cpp

#include <Arduino.h>
#include "loop.h"
#include "../utils/log.h"
#include "../control/control.h"

const int LOOP_INTERVAL_MS = 20; // 50Hz

static unsigned long lastUpdate = 0;
static int counter = 0;

void initLoop() {
    log(INFO, "Loop initialized");
}

void updateLoop() {
    unsigned long now = millis();

    if (now - lastUpdate >= LOOP_INTERVAL_MS) {
        // Keep cadence steady, but avoid burst catch-up after long pauses.
        if (now - lastUpdate > LOOP_INTERVAL_MS * 2) {
            lastUpdate = now;
        } else {
            lastUpdate += LOOP_INTERVAL_MS;
        }

        counter++;
        if (counter % 50 == 0) {
            logf(INFO, "HeartBeat: %lu", lastUpdate/1000);
        }
        update();
    }
}
