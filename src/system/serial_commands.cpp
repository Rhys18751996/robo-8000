// src/system/serial_commands.cpp
#include <Arduino.h>

#include "serial_commands.h"

#include "loop.h"
#include "mode_manager.h"

#include "../control/control.h"
#include "../input/input.h"
#include "../utils/log.h"
#include "../config/preferences_storage.h"

namespace {

// ---------- Command type ----------
using CommandFn = void(*)(const String& args);

struct Command {
    const char* name;
    CommandFn fn;
};

// ---------- Helpers ----------
void logAvailableSerialCommands();

void cmd_show_config(const String&) {
    const AppConfig config = loadPreferencesConfig();
    logPreferencesConfig(config);
}

void cmd_heartbeat(const String& args) {
    setHeartbeatLoggingEnabled(args == "on");
}

void cmd_input(const String& args) {
    setInputSnapshotLoggingEnabled(args == "on");
}

void cmd_buttons(const String& args) {
    setButtonChangeLoggingEnabled(args == "on");
}

void cmd_intent(const String& args) {
    setIntentLoggingEnabled(args == "on");
}

void cmd_battery_log(const String& args) {
    setBatteryLoggingEnabled(args == "on");
}

void cmd_show_battery(const String&) {
    const int battery = readControllerBatteryPercent();
    if (battery >= 0) {
        logf(INFO, "Controller battery: %d%%", battery);
    } else {
        log(WARN, "Controller battery unavailable (no connected controller)");
    }
}


void cmd_mode(const String& args) {
    if (args.equalsIgnoreCase("config")) {
        setSystemMode(CONFIG);
        logSystemMode();
    } else if (args.equalsIgnoreCase("run")) {
        setSystemMode(RUN);
        logSystemMode();
    } else if (args.equalsIgnoreCase("toggle")) {
        toggleSystemMode();
        logSystemMode();
    } else if (args.equalsIgnoreCase("show") || args.length() == 0) {
        logSystemMode();
    } else {
        log(WARN, "Usage: mode config|run|toggle|show");
    }
}

void cmd_show_logs(const String&) {
    logf(INFO, "heartbeat=%s input=%s buttons=%s intent=%s battery=%s",
         isHeartbeatLoggingEnabled() ? "ON" : "OFF",
         isInputSnapshotLoggingEnabled() ? "ON" : "OFF",
         isButtonChangeLoggingEnabled() ? "ON" : "OFF",
         isIntentLoggingEnabled() ? "ON" : "OFF",
         isBatteryLoggingEnabled() ? "ON" : "OFF");

    logAvailableSerialCommands();
}

// ---------- Command table ----------
Command commands[] = {
    {"show_config",   cmd_show_config},
    {"heartbeat",     cmd_heartbeat},     // usage: heartbeat on/off
    {"input",         cmd_input},         // usage: input on/off
    {"buttons",       cmd_buttons},       // usage: buttons on/off
    {"intent",        cmd_intent},        // usage: intent on/off
    {"battery_log",   cmd_battery_log},   // usage: battery_log on/off
    {"show_battery",  cmd_show_battery},
    {"show_logs",     cmd_show_logs},
    {"mode",          cmd_mode},          // usage: mode config/run/toggle/show
};

const size_t commandCount = sizeof(commands) / sizeof(commands[0]);

// ---------- Logging ----------
void logAvailableSerialCommands() {
    log(INFO, "Available commands:");
    log(INFO, "  show_config");
    log(INFO, "  heartbeat on/off");
    log(INFO, "  input on/off");
    log(INFO, "  buttons on/off");
    log(INFO, "  intent on/off");
    log(INFO, "  battery_log on/off");
    log(INFO, "  show_battery");
    log(INFO, "  show_logs");
    log(INFO, "  mode config/run/toggle/show");
}

// ---------- Parser ----------
void executeCommand(const String& input) {
    int spaceIndex = input.indexOf(' ');

    String cmd = (spaceIndex == -1) ? input : input.substring(0, spaceIndex);
    String args = (spaceIndex == -1) ? "" : input.substring(spaceIndex + 1);

    cmd.trim();
    args.trim();

    for (size_t i = 0; i < commandCount; i++) {
        if (cmd.equalsIgnoreCase(commands[i].name)) {
            commands[i].fn(args);
            return;
        }
    }

    logf(WARN, "Unknown command: %s", input.c_str());
    logAvailableSerialCommands();
}

// ---------- Serial buffer ----------
String buffer;

} // namespace

// ---------- Public API ----------
void initSerialCommands() {
    // nothing for now
}

void updateSerialCommands() {
    while (Serial.available() > 0) {
        char c = (char)Serial.read();

        if (c == '\n' || c == '\r') {
            if (buffer.length() > 0) {
                buffer.trim();
                executeCommand(buffer);
                buffer = "";
            }
        } else {
            buffer += c;
        }
    }
}