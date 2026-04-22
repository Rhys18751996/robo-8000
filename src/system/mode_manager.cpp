#include "mode_manager.h"

#include <Arduino.h>
#include <WiFi.h>

#include "../config/preferences_storage.h"
#include "../input/input.h"
#include "../utils/log.h"
#include "../web/config_server.h"

namespace {
constexpr uint8_t kModeSwitchPin = 33;       // input, LOW=CONFIG HIGH=RUN (with pullup)
constexpr uint8_t kModeLedPin = 2;           // output LED, HIGH when RUN mode
constexpr unsigned long kDebounceMs = 200;

Mode currentMode = CONFIG;
bool switchStableState = true;
bool lastRawSwitchState = true;
unsigned long lastRawChangeMs = 0;
bool inputInitialized = false;

const char* modeToString(Mode mode) {
    return mode == CONFIG ? "CONFIG" : "RUN";
}

bool readSwitchRaw() {
    return digitalRead(kModeSwitchPin) == HIGH;
}

Mode switchStateToMode(bool switchHigh) {
    return switchHigh ? RUN : CONFIG;
}

void setLedForMode(Mode mode) {
    digitalWrite(kModeLedPin, mode == RUN ? HIGH : LOW);
}

void startRunServices() {
    stopConfigServer();

    if (!inputInitialized) {
        initInput();
        inputInitialized = true;
    }

    const AppConfig cfg = loadPreferencesConfig();
    WiFi.mode(WIFI_STA);
    if (!cfg.wifiSsid.isEmpty()) {
        WiFi.begin(cfg.wifiSsid.c_str(), cfg.wifiPassword.c_str());
        logf(INFO, "RUN mode: connecting WiFi SSID '%s'", cfg.wifiSsid.c_str());
    } else {
        log(INFO, "RUN mode: WiFi SSID empty, station not started");
    }

    log(INFO, "RUN mode: Bluetooth input active");
}

void startConfigServices() {
    WiFi.disconnect(true, true);
    startConfigServer();
    log(INFO, "CONFIG mode: Bluetooth input disabled");
}

void applyMode(Mode newMode, const char* reason) {
    if (newMode == currentMode) return;

    if (newMode == CONFIG) {
        startConfigServices();
    } else {
        startRunServices();
    }

    currentMode = newMode;
    setLedForMode(currentMode);
    logf(INFO, "System mode -> %s (%s)", modeToString(currentMode), reason);
}
} // namespace

void initModeManager() {
    pinMode(kModeSwitchPin, INPUT_PULLUP);
    pinMode(kModeLedPin, OUTPUT);

    switchStableState = readSwitchRaw();
    lastRawSwitchState = switchStableState;
    lastRawChangeMs = millis();

    currentMode = switchStateToMode(switchStableState);
    setLedForMode(currentMode);

    if (currentMode == CONFIG) {
        startConfigServices();
    } else {
        startRunServices();
    }

    logf(INFO, "Mode manager initialized. Current mode=%s", currentMode == CONFIG ? "CONFIG" : "RUN");
}

void updateModeManager() {
    const unsigned long now = millis();
    const bool raw = readSwitchRaw();

    if (raw != lastRawSwitchState) {
        lastRawSwitchState = raw;
        lastRawChangeMs = now;
    }

    if (raw != switchStableState && (now - lastRawChangeMs) >= kDebounceMs) {
        switchStableState = raw;
        applyMode(switchStateToMode(switchStableState), "gpio switch");
    }

    updateConfigServer();
}

Mode getSystemMode() {
    return currentMode;
}

void setSystemMode(Mode mode) {
    applyMode(mode, "serial command");
}

void toggleSystemMode() {
    applyMode(currentMode == CONFIG ? RUN : CONFIG, "serial command toggle");
}

void logSystemMode() {
    logf(INFO, "Current system mode: %s", currentMode == CONFIG ? "CONFIG" : "RUN");
}

bool isRunMode() {
    return currentMode == RUN;
}
