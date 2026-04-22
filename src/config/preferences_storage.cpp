#include "preferences_storage.h"

#include <ArduinoJson.h> //ArduinoJson by Benoit Blanchon
#include <Preferences.h>

#include "mapping_json.h"
#include "../utils/log.h"

namespace {
constexpr const char* kPrefsNamespace = "robo8000";
constexpr const char* kKeyWifiSsid = "wifi_ssid";
constexpr const char* kKeyWifiPass = "wifi_pass";
constexpr const char* kKeyApiEndpoint = "api_endpoint";
constexpr const char* kKeyMappingJson = "mapping_json";

// Fallback API endpoint used when none has been saved yet.
// 192.168.4.1 is commonly used as the device/AP gateway during config mode.
constexpr const char* kDefaultApiEndpoint = "http://192.168.4.1/api/intent";

Preferences prefs;
bool prefsInitialized = false;

bool isEndpointValid(const String& endpoint) {
    return endpoint.startsWith("http://") || endpoint.startsWith("https://");
}

bool isMappingJsonValid(const String& mappingJson) {
    StaticJsonDocument<896> doc;
    const DeserializationError err = deserializeJson(doc, mappingJson);
    if (err) return false;

    return doc["axes"].is<JsonObject>() &&
           doc["buttons"].is<JsonObject>() &&
           doc["filters"].is<JsonObject>();
}

bool validateOrApplyFallback(AppConfig& config) {
    bool allValid = true;
    const AppConfig defaults = defaultAppConfig();

    if (config.wifiSsid.length() > 32) {
        log(WARN, "Invalid WiFi SSID length in NVS, using default");
        config.wifiSsid = defaults.wifiSsid;
        allValid = false;
    }

    if (config.wifiPassword.length() > 63) {
        log(WARN, "Invalid WiFi password length in NVS, using default");
        config.wifiPassword = defaults.wifiPassword;
        allValid = false;
    }

    if (!isEndpointValid(config.apiEndpoint)) {
        log(WARN, "Invalid API endpoint in NVS, using default");
        config.apiEndpoint = defaults.apiEndpoint;
        allValid = false;
    }

    if (!isMappingJsonValid(config.mappingJson)) {
        log(WARN, "Invalid mapping JSON in NVS, using default");
        config.mappingJson = defaults.mappingJson;
        allValid = false;
    }

    return allValid;
}
}

void initPreferencesStorage() {
    if (prefsInitialized) return;

    prefsInitialized = prefs.begin(kPrefsNamespace, false);
    if (prefsInitialized) {
        log(INFO, "Preferences initialized");
    } else {
        log(ERROR, "Failed to initialize Preferences");
    }
}

// wifiSsid is the target Wi‑Fi network SSID the robo-8000 will try to join when in RUN mode
AppConfig defaultAppConfig() {
    AppConfig config;
    config.wifiSsid = "";
    config.wifiPassword = "";
    config.apiEndpoint = kDefaultApiEndpoint;
    config.mappingJson = kDefaultMappingJson;
    return config;
}

AppConfig loadPreferencesConfig() {
    const AppConfig defaults = defaultAppConfig();

    if (!prefsInitialized) {
        log(WARN, "Preferences not initialized, using defaults");
        return defaults;
    }

    AppConfig config;
    config.wifiSsid = prefs.getString(kKeyWifiSsid, defaults.wifiSsid);
    config.wifiPassword = prefs.getString(kKeyWifiPass, defaults.wifiPassword);
    config.apiEndpoint = prefs.getString(kKeyApiEndpoint, defaults.apiEndpoint);
    config.mappingJson = prefs.getString(kKeyMappingJson, defaults.mappingJson);

    const bool allValid = validateOrApplyFallback(config);
    if (allValid) {
        log(INFO, "Configuration loaded from NVS");
    } else {
        log(WARN, "Loaded config required fallback defaults");
    }

    return config;
}

bool savePreferencesConfig(const AppConfig& config) {
    if (!prefsInitialized) {
        log(ERROR, "Cannot save config, Preferences not initialized");
        return false;
    }

    AppConfig copy = config;
    validateOrApplyFallback(copy);

    const bool okSsid = prefs.putString(kKeyWifiSsid, copy.wifiSsid) > 0 || copy.wifiSsid.isEmpty();
    const bool okPass = prefs.putString(kKeyWifiPass, copy.wifiPassword) > 0 || copy.wifiPassword.isEmpty();
    const bool okEndpoint = prefs.putString(kKeyApiEndpoint, copy.apiEndpoint) > 0;
    const bool okMapping = prefs.putString(kKeyMappingJson, copy.mappingJson) > 0;

    if (okSsid && okPass && okEndpoint && okMapping) {
        log(INFO, "Configuration saved to NVS");
        return true;
    }

    log(ERROR, "Failed to save one or more config keys");
    return false;
}


void logPreferencesConfig(const AppConfig& config) {
    log(INFO, "--- Active Preferences Config ---");
    logf(INFO, "wifiSsid: %s", config.wifiSsid.c_str());
    logf(INFO, "wifiPassword: %s", config.wifiPassword.c_str());
    logf(INFO, "apiEndpoint: %s", config.apiEndpoint.c_str());
    logf(INFO, "mappingJson: %s", config.mappingJson.c_str());
    log(INFO, "--- End Preferences Config ---");
}
