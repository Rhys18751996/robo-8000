// src/utils/loop_profiler.cpp
#include <Arduino.h>
#include "../utils/log.h"

namespace {
    uint32_t startUs = 0;

    unsigned long lastLog = 0;
    uint32_t minUs = UINT32_MAX;
    uint32_t maxUs = 0;
    uint64_t totalUs = 0;
    uint32_t count = 0;
}

void beginLoopProfile() {
    startUs = micros();
}

void endLoopProfile() {
    uint32_t duration = micros() - startUs;

    if (duration < minUs) minUs = duration;
    if (duration > maxUs) maxUs = duration;
    totalUs += duration;
    count++;

    unsigned long now = millis();
    if (now - lastLog >= 2000 && count > 0) {
        uint32_t avgUs = totalUs / count;

        logf(INFO, "Loop us: avg=%lu min=%lu max=%lu", avgUs, minUs, maxUs);

        minUs = UINT32_MAX;
        maxUs = 0;
        totalUs = 0;
        count = 0;
        lastLog = now;
    }
}