// src/input/GamepadManager.cpp
#include "GamepadManager.h"

#include <Arduino.h>

ControllerPtr GamepadManager::controllers[BP32_MAX_GAMEPADS] = {nullptr};

float GamepadManager::clamp01(float v) {
    if (v < 0.0f) return 0.0f;
    if (v > 1.0f) return 1.0f;
    return v;
}

float GamepadManager::clamp11(float v) {
    if (v < -1.0f) return -1.0f;
    if (v > 1.0f) return 1.0f;
    return v;
}

float GamepadManager::normAxis(int v) {
    return clamp11(v / 512.0f);
}

float GamepadManager::normTrigger(int v) {
    return clamp01(v / 1023.0f);
}

void GamepadManager::onConnectedController(ControllerPtr ctl) {
    for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
        if (controllers[i] == nullptr) {
            Serial.printf("[BP32] Connected idx=%d\n", i);
            controllers[i] = ctl;
            return;
        }
    }

    Serial.println("[BP32] No free slot");
}

void GamepadManager::onDisconnectedController(ControllerPtr ctl) {
    for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
        if (controllers[i] == ctl) {
            Serial.printf("[BP32] Disconnected idx=%d\n", i);
            controllers[i] = nullptr;
            return;
        }
    }
}

void GamepadManager::setup() {
    BP32.setup(&onConnectedController, &onDisconnectedController);
    BP32.enableVirtualDevice(false);

    for (auto& controller : controllers) {
        controller = nullptr;
    }

    Serial.println("[BP32] Ready");
}

ControllerPtr GamepadManager::getActiveController() {
    for (auto ctl : controllers) {
        if (ctl && ctl->isConnected() && ctl->isGamepad()) {
            return ctl;
        }
    }

    return nullptr;
}

RawInput GamepadManager::read() {
    RawInput input = {};
    input.batteryPercent = -1;

    BP32.update();

    ControllerPtr ctl = getActiveController();

    input.connected = (ctl != nullptr);
    input.state = input.connected ? ControllerState::Connected : ControllerState::Disconnected;

    if (!ctl) return input;

    input.leftStickX = normAxis(ctl->axisX());
    input.leftStickY = -normAxis(ctl->axisY());
    input.rightStickX = normAxis(ctl->axisRX());
    input.rightStickY = -normAxis(ctl->axisRY());

    input.leftTrigger = normTrigger(ctl->brake());
    input.rightTrigger = normTrigger(ctl->throttle());

    input.A = ctl->a();
    input.B = ctl->b();
    input.X = ctl->x();
    input.Y = ctl->y();

    input.LB = ctl->l1();
    input.RB = ctl->r1();

    input.leftStickClick = ctl->thumbL();
    input.rightStickClick = ctl->thumbR();

    const uint8_t dpad = ctl->dpad();
    input.dpadUp = dpad & DPAD_UP;
    input.dpadDown = dpad & DPAD_DOWN;
    input.dpadLeft = dpad & DPAD_LEFT;
    input.dpadRight = dpad & DPAD_RIGHT;

    const uint16_t misc = ctl->miscButtons();
    input.start = misc & MISC_BUTTON_START;
    input.back = misc & MISC_BUTTON_SELECT;
    input.guide = misc & MISC_BUTTON_SYSTEM;
    input.batteryPercent = ctl->battery();

    return input;
}

int GamepadManager::readBatteryPercent() {
    BP32.update();
    ControllerPtr ctl = getActiveController();
    if (!ctl) return -1;
    return ctl->battery();
}
