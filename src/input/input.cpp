// src/input/input.cpp

#include "input.h"
#include "GamepadManager.h"

namespace {
bool g_inputInitialized = false;
}

void initInput() {
    if (g_inputInitialized) return;
    GamepadManager::setup();
    g_inputInitialized = true;
}

RawInput readInput() {
    if (!g_inputInitialized) {
        RawInput input = {};
        input.batteryPercent = -1;
        input.connected = false;
        input.state = ControllerState::Disconnected;
        return input;
    }
    return GamepadManager::read();
}

int readControllerBatteryPercent() {
    if (!g_inputInitialized) return -1;
    return GamepadManager::readBatteryPercent();
}
