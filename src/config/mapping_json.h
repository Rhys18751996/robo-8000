#pragma once

// Phase 2 default mapping (config-only JSON)
constexpr const char* kDefaultMappingJson = R"json(
{
  "axes": {
    "linear": "leftStickY",
    "angular": "leftStickX"
  },
  "buttons": {
    "boost": "RB",
    "stop": "B"
  },
  "actions": {
    "pulse": "Y",
    "toggle": "X",
    "hold": "A"
  },
  "outputs": {
    "pulsePin": 2,
    "togglePin": 4,
    "holdPin": 5,
    "pulseDurationMs": 120
  },
  "filters": {
    "deadzone": 0.05,
    "smoothingAlpha": 0.20
  },
  "invert": {
    "linear": false,
    "angular": false
  }
}
)json";
