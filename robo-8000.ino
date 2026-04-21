// robo-8000.ino

#include <Arduino.h>

#include "src/control/control.h"
#include "src/input/input.h"
#include "src/system/loop.h"
#include "src/utils/log.h"
#include "src/config/preferences_storage.h"

namespace {
void logAvailableSerialCommands() {
    log(INFO, "Available commands:");
    log(INFO, "  show_config");
    log(INFO, "  heartbeat_on / heartbeat_off");
    log(INFO, "  input_on / input_off");
    log(INFO, "  buttons_on / buttons_off");
    log(INFO, "  intent_on / intent_off");
    log(INFO, "  battery_on / battery_off");
    log(INFO, "  show_battery");
    log(INFO, "  show_logs");
}

void handleSerialCommands() {
    static String buffer;

    while (Serial.available() > 0) {
        const char c = static_cast<char>(Serial.read());

        if (c == '\n' || c == '\r') {
            if (buffer.length() == 0) {
                continue;
            }

            buffer.trim();
            if (buffer == "show_config") {
                const AppConfig config = loadPreferencesConfig();
                logPreferencesConfig(config);
            } else if (buffer == "heartbeat_on") {
                setHeartbeatLoggingEnabled(true);
            } else if (buffer == "heartbeat_off") {
                setHeartbeatLoggingEnabled(false);
            } else if (buffer == "input_on") {
                setInputSnapshotLoggingEnabled(true);
            } else if (buffer == "input_off") {
                setInputSnapshotLoggingEnabled(false);
            } else if (buffer == "buttons_on") {
                setButtonChangeLoggingEnabled(true);
            } else if (buffer == "buttons_off") {
                setButtonChangeLoggingEnabled(false);
            } else if (buffer == "intent_on") {
                setIntentLoggingEnabled(true);
            } else if (buffer == "intent_off") {
                setIntentLoggingEnabled(false);
            } else if (buffer == "battery_on") {
                setBatteryLoggingEnabled(true);
            } else if (buffer == "battery_off") {
                setBatteryLoggingEnabled(false);
            } else if (buffer == "show_battery") {
                const int battery = readControllerBatteryPercent();
                if (battery >= 0) {
                    logf(INFO, "Controller battery: %d%%", battery);
                } else {
                    log(WARN, "Controller battery unavailable (no connected controller)");
                }
            } else if (buffer == "show_logs") {
                logf(INFO, "heartbeat=%s input=%s buttons=%s intent=%s battery=%s",
                     isHeartbeatLoggingEnabled() ? "ON" : "OFF",
                     isInputSnapshotLoggingEnabled() ? "ON" : "OFF",
                     isButtonChangeLoggingEnabled() ? "ON" : "OFF",
                     isIntentLoggingEnabled() ? "ON" : "OFF",
                     isBatteryLoggingEnabled() ? "ON" : "OFF");
                logAvailableSerialCommands();
            } else {
                logf(WARN, "Unknown command: %s", buffer.c_str());
                logAvailableSerialCommands();
            }

            buffer = "";
            continue;
        }

        buffer += c;
    }
}
}

void setup() {
    Serial.begin(115200);
    log(INFO, "System starting");
    initControl();
    initLoop();
}

void loop() {
    updateLoop();
    handleSerialCommands();
}
