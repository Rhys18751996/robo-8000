// src/webClient/network.cpp
#include "network.h"

#include <WiFi.h>
#include <HTTPClient.h>

#include "../config/preferences_storage.h"
#include "../utils/log.h"

namespace {
    QueueHandle_t queue;
    const int QUEUE_SIZE = 5;
    const int SEND_INTERVAL_MS = 2000;
    
    void networkTask(void* param) {
        unsigned long lastSend = 0;

        for (;;) {
            String payload;

            // Non-blocking check for latest payload
            if (xQueueReceive(queue, &payload, 0) == pdTRUE) {
                unsigned long now = millis();

                if (now - lastSend >= SEND_INTERVAL_MS) {
                    lastSend = now;

                    AppConfig cfg = loadPreferencesConfig();
                    if (cfg.apiEndpoint.length() == 0) continue;

                    HTTPClient http;
                    http.begin(cfg.apiEndpoint);
                    http.addHeader("Content-Type", "application/json");

                    http.POST(payload); // fire-and-forget (ignore result)
                    http.end();
                }
            }

            vTaskDelay(pdMS_TO_TICKS(10)); // small yield
        }
    }
}

void initWebClient() {
    queue = xQueueCreate(QUEUE_SIZE, sizeof(String));

    xTaskCreatePinnedToCore(
        networkTask,
        "networkTask",
        4096,
        nullptr,
        1,
        nullptr,
        1 // run on core 1 (keeps WiFi happier)
    );

    log(INFO, "Network task started");
}

void queueTelemetrySend(const String& payload) {
    if (!queue) return;

    // overwrite oldest if full (don’t block)
    xQueueOverwrite(queue, &payload);
}