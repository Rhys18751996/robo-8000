// src/config/preferences_storage.h
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

// Debug helper: prints the full active config to Serial logs.
void logPreferencesConfig(const AppConfig& config);
