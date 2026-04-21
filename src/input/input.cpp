// src/input/input.cpp

#include "input.h"
#include "GamepadManager.h"

void initInput() {
    GamepadManager::setup();
}

RawInput readInput() {
    return GamepadManager::read();
}

int readControllerBatteryPercent() {
    return GamepadManager::readBatteryPercent();
}
