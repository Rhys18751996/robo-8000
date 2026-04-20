#include <Arduino.h>

#include "../system/types.h"
#include "../input/input.h"
#include "../mapping/mapping.h"
#include "../utils/log.h"

RawInput input;
Intent currentIntent;
static ControllerState lastState = ControllerState::Disconnected;

namespace {
// Edge detection + output state memory for GPIO actions.
bool lastPulseAction = false;
bool lastToggleAction = false;
bool togglePinState = false;
unsigned long pulsePinDeactivateAtMs = 0;
}

const char* controllerStateToString(ControllerState state) {
    switch (state) {
        case ControllerState::Disconnected: return "Disconnected";
        case ControllerState::Connecting: return "Connecting";
        case ControllerState::Connected: return "Connected";
        default: return "Unknown";
    }
}

void initControl() {
    initInput();
    initMapping();

    // Configure output pins from mapping config (no hardcoded pins here).
    const MappingConfig& mapCfg = getMappingConfig();
    pinMode(mapCfg.pulsePin, OUTPUT);
    pinMode(mapCfg.togglePin, OUTPUT);
    pinMode(mapCfg.holdPin, OUTPUT);
    digitalWrite(mapCfg.pulsePin, LOW);
    digitalWrite(mapCfg.togglePin, LOW);
    digitalWrite(mapCfg.holdPin, LOW);

    logf(INFO, "Output pins initialized: pulse=%u toggle=%u hold=%u", mapCfg.pulsePin,
         mapCfg.togglePin, mapCfg.holdPin);
}

void readInputs() {
    input = readInput();

    // Log only state transitions to avoid serial spam.
    if (input.state != lastState) {
        logf(INFO, "Controller state: %s", controllerStateToString(input.state));
        lastState = input.state;
    }
}

void mapToIntent() {
    currentIntent = mapInputToIntent(input);
}

void applySafety() {
    // Safety currently centralized in mapInputToIntent().
    // Keeping this function in the loop preserves Phase 0 architecture shape.
}

void outputControl() {
    const MappingConfig& mapCfg = getMappingConfig();

    // Pulse output: goes HIGH once on button press, then auto-LOW after duration.
    if (currentIntent.pulseAction && !lastPulseAction) {
        digitalWrite(mapCfg.pulsePin, HIGH);
        pulsePinDeactivateAtMs = millis() + mapCfg.pulseDurationMs;
    }

    if (pulsePinDeactivateAtMs != 0 &&
        static_cast<long>(millis() - pulsePinDeactivateAtMs) >= 0) {
        digitalWrite(mapCfg.pulsePin, LOW);
        pulsePinDeactivateAtMs = 0;
    }

    // Toggle output: flips state only on rising edge.
    if (currentIntent.toggleAction && !lastToggleAction) {
        togglePinState = !togglePinState;
        digitalWrite(mapCfg.togglePin, togglePinState ? HIGH : LOW);
    }

    // Hold output: directly mirrors button state.
    digitalWrite(mapCfg.holdPin, currentIntent.holdAction ? HIGH : LOW);

    // Store current states for next-loop edge detection.
    lastPulseAction = currentIntent.pulseAction;
    lastToggleAction = currentIntent.toggleAction;
}

void update() {
    readInputs();
    mapToIntent();
    applySafety();

    static int debugCounter = 0;
    debugCounter++;

    if (debugCounter % 50 == 0) {
        logf(INFO, "Intent L: %.2f A: %.2f Conn: %d", currentIntent.linear,
             currentIntent.angular, input.connected);
    }

    outputControl();
}
