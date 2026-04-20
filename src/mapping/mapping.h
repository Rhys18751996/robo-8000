#pragma once

#include <stdint.h>

#include "../system/types.h"
#include "../input/input.h"

// Runtime mapping configuration loaded from JSON at startup.
//
// Think of this as "how to translate controller input into behavior".
// You can change button/axis names and output pins without changing control loop code.
struct MappingConfig {
    // Axis mapping used for driving intent.
    const char* linearAxis;
    const char* angularAxis;

    // Basic behavior buttons.
    const char* boostButton;
    const char* stopButton;

    // GPIO action buttons.
    const char* pulseButton;
    const char* toggleButton;
    const char* holdButton;

    // GPIO pins used by pulse/toggle/hold actions.
    uint8_t pulsePin;
    uint8_t togglePin;
    uint8_t holdPin;
    uint16_t pulseDurationMs;  // how long pulsePin stays HIGH after press

    // Input filtering options.
    float deadzone;
    float smoothingAlpha;
    bool invertLinear;
    bool invertAngular;

    // Reference lists of currently supported Xbox mapping names.
    // These are informational and can be surfaced later in config UI/docs.
    const char* availableAxes[6];
    const char* availableButtons[15];
};

// Parse JSON once and populate MappingConfig defaults/overrides.
void initMapping();

// Main translation function: RawInput -> Intent.
Intent mapInputToIntent(const RawInput& input);

// Exposes active mapping config for modules like control output setup.
const MappingConfig& getMappingConfig();
