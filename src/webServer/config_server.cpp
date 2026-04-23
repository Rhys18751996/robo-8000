// src/webServer/config_server.cpp
#include "config_server.h"

#include <Arduino.h>
#include <WebServer.h>
#include <WiFi.h>

#include "../config/preferences_storage.h"
#include "../utils/log.h"

namespace {
// The AP SSID your phone connects to when robo-8000 is in CONFIG mode is hardcoded as "RobotConfig"

// Once your phone connects to the ESP32 AP (RobotConfig), use:
// http://192.168.4.1/ → config page (GET /) 
// http://192.168.4.1/save → save config (POST /save)
constexpr const char* kApSsid = "RobotConfig";

WebServer server(80);
bool serverRunning = false;

String htmlEscape(const String& in) {
    String out;
    out.reserve(in.length() + 16);
    for (size_t i = 0; i < in.length(); i++) {
        const char c = in[i];
        switch (c) {
            case '&': out += F("&amp;"); break;
            case '<': out += F("&lt;"); break;
            case '>': out += F("&gt;"); break;
            case '"': out += F("&quot;"); break;
            default: out += c; break;
        }
    }
    return out;
}

String buildConfigPage(const String& statusMessage) {
    const AppConfig cfg = loadPreferencesConfig();

    String page;
    page.reserve(3500);
    page += F("<!doctype html><html><head><meta name='viewport' content='width=device-width,initial-scale=1'>");
    page += F("<title>Robot Config</title><style>body{font-family:sans-serif;max-width:760px;margin:16px auto;padding:0 12px;}label{display:block;margin-top:12px;font-weight:600;}input,textarea{width:100%;padding:8px;box-sizing:border-box;}button{margin-top:14px;padding:10px 14px;}pre{white-space:pre-wrap;background:#f5f5f5;padding:8px;}</style></head><body>");
    page += F("<h2>Robot Config</h2>");
    if (statusMessage.length() > 0) {
        page += F("<p><b>");
        page += htmlEscape(statusMessage);
        page += F("</b></p>");
    }
    page += F("<form method='POST' action='/save'>");
    page += F("<label>WiFi SSID</label><input name='wifiSsid' value='");
    page += htmlEscape(cfg.wifiSsid);
    page += F("'>");
    page += F("<label>WiFi Password</label><input type='password' name='wifiPassword' value='");
    page += htmlEscape(cfg.wifiPassword);
    page += F("'>");
    page += F("<label>API Endpoint</label><input name='apiEndpoint' value='");
    page += htmlEscape(cfg.apiEndpoint);
    page += F("'>");
    page += F("<label>Mapping JSON</label><textarea name='mappingJson' rows='14'>");
    page += htmlEscape(cfg.mappingJson);
    page += F("</textarea>");
    page += F("<button type='submit'>Save</button></form>");
    page += F("</body></html>");
    return page;
}

void handleRoot() {
    server.send(200, "text/html", buildConfigPage(""));
}

void handleSave() {
    AppConfig cfg = loadPreferencesConfig();
    if (server.hasArg("wifiSsid")) cfg.wifiSsid = server.arg("wifiSsid");
    if (server.hasArg("wifiPassword")) cfg.wifiPassword = server.arg("wifiPassword");
    if (server.hasArg("apiEndpoint")) cfg.apiEndpoint = server.arg("apiEndpoint");
    if (server.hasArg("mappingJson")) cfg.mappingJson = server.arg("mappingJson");

    const bool ok = savePreferencesConfig(cfg);
    const String msg = ok ? "Saved configuration to NVS" : "Save failed (check logs)";
    logf(INFO, "Web config save result: %s", ok ? "OK" : "FAIL");
    server.send(ok ? 200 : 500, "text/html", buildConfigPage(msg));
}

void registerRoutes() {
    server.on("/", HTTP_GET, handleRoot);
    server.on("/save", HTTP_POST, handleSave);
    server.onNotFound([]() { server.send(404, "text/plain", "Not found"); });
}
} // namespace

void startConfigServer() {
    if (serverRunning) return;

    WiFi.mode(WIFI_AP_STA);
    if (WiFi.softAP(kApSsid)) {
        IPAddress ip = WiFi.softAPIP();
        logf(INFO, "Config AP started: SSID=%s IP=%s", kApSsid, ip.toString().c_str());
    } else {
        log(ERROR, "Failed to start config AP");
    }

    registerRoutes();
    server.begin();
    serverRunning = true;
    log(INFO, "Config web server started");
}

void stopConfigServer() {
    if (!serverRunning) return;

    server.stop();
    WiFi.softAPdisconnect(true);
    serverRunning = false;
    log(INFO, "Config web server stopped");
}

void updateConfigServer() {
    if (!serverRunning) return;
    server.handleClient();
}

bool isConfigServerRunning() {
    return serverRunning;
}
