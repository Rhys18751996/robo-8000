// robo-8000.ino

#include <Arduino.h>

#include "src/control/control.h"
#include "src/system/loop.h"
#include "src/utils/log.h"
#include "src/config/preferences_storage.h"

namespace {
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
            } else {
                logf(WARN, "Unknown command: %s", buffer.c_str());
                log(INFO, "Available commands: show_config");
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
