#pragma once

#include <Arduino.h>

// Runtime configuration persisted in ESP32 NVS.
struct AppConfig {
    String wifiSsid;
    String wifiPassword;
    String apiEndpoint;
    String mappingJson;
};

void initPreferencesStorage();
AppConfig loadPreferencesConfig();
bool savePreferencesConfig(const AppConfig& config);

// Safe defaults used when preferences are missing or invalid.
AppConfig defaultAppConfig();
