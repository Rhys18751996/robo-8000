// src/system/types.h

#pragma once

// High-level command object used by the rest of the system.
//
// This is intentionally "controller-agnostic": the mapping layer translates
// specific controller buttons/sticks into this generic intent.
struct Intent {
    float linear;    // forward/backward command range: -1.0 → 1.0
    float angular;   // turn command range: -1.0 → 1.0
    bool boost;      // optional speed modifier action
    bool stop;       // emergency stop style action

    // Phase 2+ output actions.
    // These are mapped from buttons and consumed by outputControl().
    bool pulseAction;   // one-shot pulse action on rising button edge
    bool toggleAction;  // toggles output state on each button press
    bool holdAction;    // output stays active while button is held
};

enum Mode {
    CONFIG,
    RUN
};

enum class ControllerState {
    Disconnected,
    Connecting,
    Connected
};
