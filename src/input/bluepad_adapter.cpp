// src/input/bluepad_adapter.cpp

#include "bluepad_adapter.h"
#include "input.h"
#include <Bluepad32.h>

static ControllerPtr controllers[BP32_MAX_GAMEPADS];

// --- Normalize ---
static float clamp01(float v) {
    if (v < 0.0f) return 0.0f;
    if (v > 1.0f) return 1.0f;
    return v;
}

static float clamp11(float v) {
    if (v < -1.0f) return -1.0f;
    if (v > 1.0f) return 1.0f;
    return v;
}

static float normAxis(int v) {
    return clamp11(v / 512.0f);
}

static float normTrigger(int v) {
    return clamp01(v / 1023.0f);
}

// --- Callbacks ---
void onConnectedController(ControllerPtr ctl) {
    for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
        if (controllers[i] == nullptr) {
            Serial.printf("[BP32] Connected idx=%d\n", i);
            controllers[i] = ctl;
            return;
        }
    }
    Serial.println("[BP32] No free slot");
}

void onDisconnectedController(ControllerPtr ctl) {
    for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
        if (controllers[i] == ctl) {
            Serial.printf("[BP32] Disconnected idx=%d\n", i);
            controllers[i] = nullptr;
            return;
        }
    }
}

// --- Init ---
void initGamepad() {
    BP32.setup(&onConnectedController, &onDisconnectedController);
    for (auto &c : controllers) c = nullptr;
    Serial.println("[BP32] Ready");
}

// --- Pick first active controller ---
static ControllerPtr getActiveController() {
    for (auto ctl : controllers) {
        if (ctl && ctl->isConnected() && ctl->isGamepad()) {
            return ctl;
        }
    }
    return nullptr;
}

// --- Read ---
RawInput readGamepad() {
    RawInput input = {};

    BP32.update();

    ControllerPtr ctl = getActiveController();

    input.connected = (ctl != nullptr);
    input.state = input.connected
        ? ControllerState::Connected
        : ControllerState::Disconnected;

    if (!ctl) return input;

    // sticks
    input.leftStickX  = normAxis(ctl->axisX());
    input.leftStickY  = -normAxis(ctl->axisY());
    input.rightStickX = normAxis(ctl->axisRX());
    input.rightStickY = -normAxis(ctl->axisRY());

    // triggers
    input.leftTrigger  = normTrigger(ctl->brake());
    input.rightTrigger = normTrigger(ctl->throttle());

    // face buttons (already Xbox mapping in BP32)
    input.A = ctl->a();
    input.B = ctl->b();
    input.X = ctl->x();
    input.Y = ctl->y();

    // shoulders
    input.LB = ctl->l1();
    input.RB = ctl->r1();

    // stick clicks
    input.leftStickClick  = ctl->thumbL();
    input.rightStickClick = ctl->thumbR();

    // dpad
    uint8_t dpad = ctl->dpad();
    input.dpadUp    = dpad & DPAD_UP;
    input.dpadDown  = dpad & DPAD_DOWN;
    input.dpadLeft  = dpad & DPAD_LEFT;
    input.dpadRight = dpad & DPAD_RIGHT;

    // system buttons
    uint16_t misc = ctl->miscButtons();
    input.start = misc & MISC_BUTTON_START;
    input.back  = misc & MISC_BUTTON_SELECT;
    input.guide = misc & MISC_BUTTON_SYSTEM;

    return input;
}
