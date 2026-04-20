#include "mapping.h"

#include <ArduinoJson.h>
#include <math.h>
#include <string.h>

#include "../utils/log.h"
#include "../config/mapping_json.h"

namespace {
// Default internal mapping values.
// These are used first, then optionally overridden by parsed JSON.
MappingConfig config = {
    "leftStickY",
    "leftStickX",
    "RB",
    "B",
    "Y",
    "X",
    "A",
    2,
    4,
    5,
    120,
    0.05f,
    0.20f,
    false,
    false,
    {"leftStickX", "leftStickY", "rightStickX", "rightStickY", "leftTrigger", "rightTrigger"},
    {"A", "B", "X", "Y", "LB", "RB", "leftStickClick", "rightStickClick", "dpadUp", "dpadDown", "dpadLeft", "dpadRight", "start", "back", "guide"},
};

// Previous smoothed values, preserved across update() iterations.
float lastLinear = 0.0f;
float lastAngular = 0.0f;

// Ignore tiny stick movement around center to prevent drift/jitter.
float applyDeadzone(float value, float deadzone) {
    return (fabs(value) < deadzone) ? 0.0f : value;
}

// First-order smoothing filter.
// Lower alpha = smoother/slower response, higher alpha = sharper response.
float smooth(float current, float previous, float alpha) {
    return alpha * current + (1.0f - alpha) * previous;
}

// Convert configured axis name to the matching RawInput field.
float readAxisByName(const RawInput& input, const char* axisName) {
    if (strcmp(axisName, "leftStickX") == 0) return input.leftStickX;
    if (strcmp(axisName, "leftStickY") == 0) return input.leftStickY;
    if (strcmp(axisName, "rightStickX") == 0) return input.rightStickX;
    if (strcmp(axisName, "rightStickY") == 0) return input.rightStickY;
    if (strcmp(axisName, "leftTrigger") == 0) return input.leftTrigger;
    if (strcmp(axisName, "rightTrigger") == 0) return input.rightTrigger;

    logf(WARN, "Unknown axis in mapping: %s", axisName);
    return 0.0f;
}

// Convert configured button name to the matching RawInput field.
bool readButtonByName(const RawInput& input, const char* buttonName) {
    if (strcmp(buttonName, "A") == 0) return input.A;
    if (strcmp(buttonName, "B") == 0) return input.B;
    if (strcmp(buttonName, "X") == 0) return input.X;
    if (strcmp(buttonName, "Y") == 0) return input.Y;
    if (strcmp(buttonName, "LB") == 0) return input.LB;
    if (strcmp(buttonName, "RB") == 0) return input.RB;
    if (strcmp(buttonName, "leftStickClick") == 0) return input.leftStickClick;
    if (strcmp(buttonName, "rightStickClick") == 0) return input.rightStickClick;
    if (strcmp(buttonName, "dpadUp") == 0) return input.dpadUp;
    if (strcmp(buttonName, "dpadDown") == 0) return input.dpadDown;
    if (strcmp(buttonName, "dpadLeft") == 0) return input.dpadLeft;
    if (strcmp(buttonName, "dpadRight") == 0) return input.dpadRight;
    if (strcmp(buttonName, "start") == 0) return input.start;
    if (strcmp(buttonName, "back") == 0) return input.back;
    if (strcmp(buttonName, "guide") == 0) return input.guide;

    logf(WARN, "Unknown button in mapping: %s", buttonName);
    return false;
}
}  // namespace

void initMapping() {
    StaticJsonDocument<896> doc;
    DeserializationError error = deserializeJson(doc, kDefaultMappingJson);

    if (error) {
        // Keep built-in defaults if JSON parse fails.
        logf(ERROR, "Mapping JSON parse failed: %s", error.c_str());
        log(WARN, "Using built-in fallback mapping values");
        return;
    }

    // Use parsed value when present, otherwise keep existing default.
    config.linearAxis = doc["axes"]["linear"] | config.linearAxis;
    config.angularAxis = doc["axes"]["angular"] | config.angularAxis;
    config.boostButton = doc["buttons"]["boost"] | config.boostButton;
    config.stopButton = doc["buttons"]["stop"] | config.stopButton;
    config.pulseButton = doc["actions"]["pulse"] | config.pulseButton;
    config.toggleButton = doc["actions"]["toggle"] | config.toggleButton;
    config.holdButton = doc["actions"]["hold"] | config.holdButton;
    config.pulsePin = doc["outputs"]["pulsePin"] | config.pulsePin;
    config.togglePin = doc["outputs"]["togglePin"] | config.togglePin;
    config.holdPin = doc["outputs"]["holdPin"] | config.holdPin;
    config.pulseDurationMs = doc["outputs"]["pulseDurationMs"] | config.pulseDurationMs;
    config.deadzone = doc["filters"]["deadzone"] | config.deadzone;
    config.smoothingAlpha = doc["filters"]["smoothingAlpha"] | config.smoothingAlpha;
    config.invertLinear = doc["invert"]["linear"] | config.invertLinear;
    config.invertAngular = doc["invert"]["angular"] | config.invertAngular;

    logf(INFO,
         "Mapping loaded: L=%s A=%s boost=%s stop=%s pulseBtn=%s toggleBtn=%s holdBtn=%s",
         config.linearAxis, config.angularAxis, config.boostButton, config.stopButton,
         config.pulseButton, config.toggleButton, config.holdButton);
}

Intent mapInputToIntent(const RawInput& input) {
    // Start from safe defaults each cycle.
    Intent intent = {0.0f, 0.0f, false, true, false, false, false};

    // 1) Read mapped axes.
    float linear = readAxisByName(input, config.linearAxis);
    float angular = readAxisByName(input, config.angularAxis);

    // 2) Apply optional inversion.
    if (config.invertLinear) linear *= -1.0f;
    if (config.invertAngular) angular *= -1.0f;

    // 3) Apply filtering.
    linear = applyDeadzone(linear, config.deadzone);
    angular = applyDeadzone(angular, config.deadzone);

    intent.linear = smooth(linear, lastLinear, config.smoothingAlpha);
    intent.angular = smooth(angular, lastAngular, config.smoothingAlpha);
    lastLinear = intent.linear;
    lastAngular = intent.angular;

    // 4) Map buttons to intent actions.
    intent.boost = readButtonByName(input, config.boostButton);
    intent.stop = readButtonByName(input, config.stopButton);
    intent.pulseAction = readButtonByName(input, config.pulseButton);
    intent.toggleAction = readButtonByName(input, config.toggleButton);
    intent.holdAction = readButtonByName(input, config.holdButton);

    // 5) Safety fallback when controller is disconnected.
    if (!input.connected) {
        intent.linear = 0.0f;
        intent.angular = 0.0f;
        intent.boost = false;
        intent.stop = true;
        intent.pulseAction = false;
        intent.toggleAction = false;
        intent.holdAction = false;
        lastLinear = 0.0f;
        lastAngular = 0.0f;
    }

    // 6) Stop button overrides movement.
    if (intent.stop) {
        intent.linear = 0.0f;
        intent.angular = 0.0f;
    }

    return intent;
}

const MappingConfig& getMappingConfig() {
    return config;
}
