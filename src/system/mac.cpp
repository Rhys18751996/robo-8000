// src/system/mac.cpp
#include "mac.h"

#include <Arduino.h>
#include <esp_system.h>

static void printMac(const char* label, esp_mac_type_t type) {
    uint8_t mac[6];
    esp_read_mac(mac, type);

    Serial.printf("%s MAC: %02X:%02X:%02X:%02X:%02X:%02X\n",
        label,
        mac[0], mac[1], mac[2],
        mac[3], mac[4], mac[5]);
}

void printMacAddresses() {
    printMac("WiFi STA", ESP_MAC_WIFI_STA);
    printMac("WiFi AP ", ESP_MAC_WIFI_SOFTAP);
    printMac("BT      ", ESP_MAC_BT);
}