#include "GamepadManager.h"
#include <Arduino.h>

ControllerPtr GamepadManager::controllers[BP32_MAX_GAMEPADS] = { nullptr };

// --- Callbacks ---
void GamepadManager::onConnectedController(ControllerPtr ctl) {
    for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
        if (controllers[i] == nullptr) {
            Serial.printf("Controller connected at index %d\n", i);
            controllers[i] = ctl;
            return;
        }
    }
    Serial.println("No free slot for controller");
}

void GamepadManager::onDisconnectedController(ControllerPtr ctl) {
    for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
        if (controllers[i] == ctl) {
            Serial.printf("Controller disconnected from index %d\n", i);
            controllers[i] = nullptr;
            return;
        }
    }
}

// --- Setup ---
void GamepadManager::setup() {
    BP32.setup(&onConnectedController, &onDisconnectedController);
    BP32.enableVirtualDevice(false); // no mouse
}

// --- Update loop ---
void GamepadManager::update() {
    BP32.update();

    for (auto ctl : controllers) {
        if (ctl && ctl->isConnected() && ctl->hasData()) {
            processGamepad(ctl);
        }
    }
}

// --- Gamepad logic only ---
void GamepadManager::processGamepad(ControllerPtr ctl) {
    // Example: print button bitmask
    Serial.printf("Buttons: 0x%04x\n", ctl->buttons());

    // Example: simple actions
    if (ctl->a()) {
        Serial.println("A pressed");
    }

    if (ctl->b()) {
        Serial.println("B pressed");
    }

    // Example: analog stick
    Serial.printf("LX: %d  LY: %d\n", ctl->axisX(), ctl->axisY());
}