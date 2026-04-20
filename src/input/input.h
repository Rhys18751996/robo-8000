// src/input/input.h

#pragma once

#include "../system/types.h"

struct RawInput {
    // --- Analog Sticks (-1.0 → 1.0) ---
    float leftStickX;
    float leftStickY;
    float rightStickX;
    float rightStickY;

    // --- Triggers (0.0 → 1.0) ---
    float leftTrigger;
    float rightTrigger;

    // --- Face Buttons (Xbox layout) ---
    bool A;
    bool B;
    bool X;
    bool Y;

    // --- Shoulder Buttons ---
    bool LB;
    bool RB;

    // --- Stick Clicks ---
    bool leftStickClick;
    bool rightStickClick;

    // --- D-Pad ---
    bool dpadUp;
    bool dpadDown;
    bool dpadLeft;
    bool dpadRight;

    // --- System Buttons ---
    bool start;     // Menu
    bool back;      // View
    bool guide;     // Xbox button

    // --- Meta ---
    bool connected;
    ControllerState state;
};

void initInput();
RawInput readInput();