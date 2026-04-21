#include <Arduino.h>
#include <string.h>

#include "../system/types.h"
#include "../input/input.h"
#include "../mapping/mapping.h"
#include "../config/preferences_storage.h"
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
bool inputSnapshotLoggingEnabled = true;
bool buttonChangeLoggingEnabled = false;
bool intentLoggingEnabled = false;
bool batteryLoggingEnabled = false;

void appendToken(char* buffer, size_t size, bool& hasAny, const char* token) {
    if (!token || !buffer || size == 0) return;
    if (hasAny) {
        strncat(buffer, ",", size - strlen(buffer) - 1);
    }
    strncat(buffer, token, size - strlen(buffer) - 1);
    hasAny = true;
}

void buildPressedButtonsText(const RawInput& in, char* out, size_t outSize) {
    if (!out || outSize == 0) return;
    out[0] = '\0';
    bool hasAny = false;

    if (in.A) appendToken(out, outSize, hasAny, "A");
    if (in.B) appendToken(out, outSize, hasAny, "B");
    if (in.X) appendToken(out, outSize, hasAny, "X");
    if (in.Y) appendToken(out, outSize, hasAny, "Y");
    if (in.LB) appendToken(out, outSize, hasAny, "LB");
    if (in.RB) appendToken(out, outSize, hasAny, "RB");
    if (in.leftStickClick) appendToken(out, outSize, hasAny, "L3");
    if (in.rightStickClick) appendToken(out, outSize, hasAny, "R3");
    if (in.dpadUp) appendToken(out, outSize, hasAny, "DPadUp");
    if (in.dpadDown) appendToken(out, outSize, hasAny, "DPadDown");
    if (in.dpadLeft) appendToken(out, outSize, hasAny, "DPadLeft");
    if (in.dpadRight) appendToken(out, outSize, hasAny, "DPadRight");
    if (in.start) appendToken(out, outSize, hasAny, "Start");
    if (in.back) appendToken(out, outSize, hasAny, "Back");
    if (in.guide) appendToken(out, outSize, hasAny, "Guide");

    if (!hasAny) {
        strlcpy(out, "(none)", outSize);
    }
}
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
    initPreferencesStorage();
    const AppConfig appConfig = loadPreferencesConfig();

    initInput();
    initMapping(appConfig.mappingJson.c_str());

    logf(INFO, "Config: WiFi SSID len=%u API=%s", appConfig.wifiSsid.length(),
         appConfig.apiEndpoint.c_str());

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

    // Readable input diagnostics:
    // - Button names are logged when the pressed-set changes.
    // - A full input snapshot is logged periodically, even when idle.
    static char lastButtonsText[160] = "";
    static unsigned long lastSnapshotMs = 0;
    char buttonsText[160];
    buildPressedButtonsText(input, buttonsText, sizeof(buttonsText));
    if (buttonChangeLoggingEnabled && strcmp(buttonsText, lastButtonsText) != 0) {
        logf(INFO, "Buttons: %s", buttonsText);
    }
    strlcpy(lastButtonsText, buttonsText, sizeof(lastButtonsText));

    const unsigned long nowMs = millis();
    if (inputSnapshotLoggingEnabled && (lastSnapshotMs == 0 || nowMs - lastSnapshotMs >= 250)) {
        lastSnapshotMs = nowMs;
        if (batteryLoggingEnabled) {
            logf(INFO,
                 "Input LX:%+.2f LY:%+.2f RX:%+.2f RY:%+.2f LT:%.2f RT:%.2f Bat:%d%% | "
                 "A:%d B:%d X:%d Y:%d LB:%d RB:%d L3:%d R3:%d U:%d D:%d L:%d R:%d Start:%d Back:%d Guide:%d",
                 input.leftStickX, input.leftStickY, input.rightStickX, input.rightStickY,
                 input.leftTrigger, input.rightTrigger, input.batteryPercent, input.A, input.B,
                 input.X, input.Y, input.LB, input.RB, input.leftStickClick,
                 input.rightStickClick, input.dpadUp, input.dpadDown, input.dpadLeft,
                 input.dpadRight, input.start, input.back, input.guide);
        } else {
            logf(INFO,
                 "Input LX:%+.2f LY:%+.2f RX:%+.2f RY:%+.2f LT:%.2f RT:%.2f | "
                 "A:%d B:%d X:%d Y:%d LB:%d RB:%d L3:%d R3:%d U:%d D:%d L:%d R:%d Start:%d Back:%d Guide:%d",
                 input.leftStickX, input.leftStickY, input.rightStickX, input.rightStickY,
                 input.leftTrigger, input.rightTrigger, input.A, input.B, input.X, input.Y,
                 input.LB, input.RB, input.leftStickClick, input.rightStickClick, input.dpadUp,
                 input.dpadDown, input.dpadLeft, input.dpadRight, input.start, input.back, input.guide);
        }
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

    if (intentLoggingEnabled && debugCounter % 50 == 0) {
        logf(INFO, "Intent L: %.2f A: %.2f Conn: %d", currentIntent.linear,
             currentIntent.angular, input.connected);
    }

    outputControl();
}

void setInputSnapshotLoggingEnabled(bool enabled) {
    inputSnapshotLoggingEnabled = enabled;
    logf(INFO, "Input snapshot logging: %s", enabled ? "ON" : "OFF");
}

bool isInputSnapshotLoggingEnabled() {
    return inputSnapshotLoggingEnabled;
}

void setButtonChangeLoggingEnabled(bool enabled) {
    buttonChangeLoggingEnabled = enabled;
    logf(INFO, "Button change logging: %s", enabled ? "ON" : "OFF");
}

bool isButtonChangeLoggingEnabled() {
    return buttonChangeLoggingEnabled;
}

void setIntentLoggingEnabled(bool enabled) {
    intentLoggingEnabled = enabled;
    logf(INFO, "Intent logging: %s", enabled ? "ON" : "OFF");
}

bool isIntentLoggingEnabled() {
    return intentLoggingEnabled;
}

void setBatteryLoggingEnabled(bool enabled) {
    batteryLoggingEnabled = enabled;
    logf(INFO, "Battery in input logs: %s", enabled ? "ON" : "OFF");
}

bool isBatteryLoggingEnabled() {
    return batteryLoggingEnabled;
}
