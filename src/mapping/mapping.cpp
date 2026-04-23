// src/mapping/mapping.cpp
#include "mapping.h"

#include <ArduinoJson.h> //ArduinoJson by Benoit Blanchon
#include <ctype.h>
#include <math.h>
#include <string.h>

#include "../utils/log.h"

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
    16,
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

bool isIgnorableNameChar(char c) {
    return c == ' ' || c == '_' || c == '-';
}

// Case-insensitive comparison with tolerant formatting:
// ignores spaces/underscores/hyphens from mapping JSON.
bool equalsNormalized(const char* a, const char* b) {
    if (!a || !b) return false;
    while (*a || *b) {
        while (*a && isIgnorableNameChar(*a)) ++a;
        while (*b && isIgnorableNameChar(*b)) ++b;
        if (!*a || !*b) break;
        if (tolower(*a) != tolower(*b)) return false;
        ++a;
        ++b;
    }
    while (*a && isIgnorableNameChar(*a)) ++a;
    while (*b && isIgnorableNameChar(*b)) ++b;
    return *a == '\0' && *b == '\0';
}

void warnUnknownOnce(const char* type, const char* name) {
    static char seen[12][24] = {};
    const char* safeName = (name && name[0] != '\0') ? name : "<empty>";

    for (size_t i = 0; i < 12; ++i) {
        if (seen[i][0] == '\0') break;
        if (strcmp(seen[i], safeName) == 0) return;
    }

    for (size_t i = 0; i < 12; ++i) {
        if (seen[i][0] == '\0') {
            strlcpy(seen[i], safeName, sizeof(seen[i]));
            break;
        }
    }

    // this spams the serial monitor even iff the buttons and inputs are all working
    //logf(WARN, "Unknown %s in mapping: %s", type, safeName);
}

// Convert configured axis name to the matching RawInput field.
float readAxisByName(const RawInput& input, const char* axisName) {
    if (equalsNormalized(axisName, "leftStickX") || equalsNormalized(axisName, "lx")) return input.leftStickX;
    if (equalsNormalized(axisName, "leftStickY") || equalsNormalized(axisName, "ly")) return input.leftStickY;
    if (equalsNormalized(axisName, "rightStickX") || equalsNormalized(axisName, "rx")) return input.rightStickX;
    if (equalsNormalized(axisName, "rightStickY") || equalsNormalized(axisName, "ry")) return input.rightStickY;
    if (equalsNormalized(axisName, "leftTrigger") || equalsNormalized(axisName, "lt")) return input.leftTrigger;
    if (equalsNormalized(axisName, "rightTrigger") || equalsNormalized(axisName, "rt")) return input.rightTrigger;

    warnUnknownOnce("axis", axisName);
    return 0.0f;
}

// Convert configured button name to the matching RawInput field.
bool readButtonByName(const RawInput& input, const char* buttonName) {
    if (equalsNormalized(buttonName, "A") || equalsNormalized(buttonName, "cross")) return input.A;
    if (equalsNormalized(buttonName, "B") || equalsNormalized(buttonName, "circle")) return input.B;
    if (equalsNormalized(buttonName, "X") || equalsNormalized(buttonName, "square")) return input.X;
    if (equalsNormalized(buttonName, "Y") || equalsNormalized(buttonName, "triangle")) return input.Y;
    if (equalsNormalized(buttonName, "LB") || equalsNormalized(buttonName, "l1")) return input.LB;
    if (equalsNormalized(buttonName, "RB") || equalsNormalized(buttonName, "r1")) return input.RB;
    if (equalsNormalized(buttonName, "leftStickClick") || equalsNormalized(buttonName, "ls") || equalsNormalized(buttonName, "thumbL")) return input.leftStickClick;
    if (equalsNormalized(buttonName, "rightStickClick") || equalsNormalized(buttonName, "rs") || equalsNormalized(buttonName, "thumbR")) return input.rightStickClick;
    if (equalsNormalized(buttonName, "dpadUp")) return input.dpadUp;
    if (equalsNormalized(buttonName, "dpadDown")) return input.dpadDown;
    if (equalsNormalized(buttonName, "dpadLeft")) return input.dpadLeft;
    if (equalsNormalized(buttonName, "dpadRight")) return input.dpadRight;
    if (equalsNormalized(buttonName, "start") || equalsNormalized(buttonName, "menu")) return input.start;
    if (equalsNormalized(buttonName, "back") || equalsNormalized(buttonName, "select") || equalsNormalized(buttonName, "view")) return input.back;
    if (equalsNormalized(buttonName, "guide") || equalsNormalized(buttonName, "system") || equalsNormalized(buttonName, "home")) return input.guide;

    warnUnknownOnce("button", buttonName);
    return false;
}
}  // namespace

void initMapping(const char* mappingJson) {
    StaticJsonDocument<896> doc;
    DeserializationError error = deserializeJson(doc, mappingJson);

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
