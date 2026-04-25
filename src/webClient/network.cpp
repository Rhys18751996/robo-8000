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

        const AppConfig cfg = loadPreferencesConfig();

        for (;;) {
            char* payload = nullptr;

            if (xQueueReceive(queue, &payload, 0) == pdTRUE) {
                unsigned long now = millis();

                if (now - lastSend >= SEND_INTERVAL_MS) {
                    lastSend = now;

                    if (cfg.apiEndpoint.length() > 0) {
                        HTTPClient http;
                        http.begin(cfg.apiEndpoint);
                        http.addHeader("Content-Type", "application/json");

                        http.POST((uint8_t*)payload, strlen(payload));
                        http.end();
                    }
                }

                free(payload);
            }

            vTaskDelay(pdMS_TO_TICKS(10));
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

    char* copy = strdup(payload.c_str());  // allocate heap copy
    if (!copy) {
        log(ERROR, "Failed to allocate payload");
        return;
    }

    // Try to send without blocking
    if (xQueueSend(queue, &copy, 0) != pdTRUE) {
        // Queue full → drop oldest and retry
        char* dropped;
        if (xQueueReceive(queue, &dropped, 0) == pdTRUE) {
            free(dropped);  // prevent leak
        }

        if (xQueueSend(queue, &copy, 0) != pdTRUE) {
            log(WARN, "Queue still full, dropping payload");
            free(copy);
        }
    }
}