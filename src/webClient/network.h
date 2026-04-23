// src/webClient/network.h
#pragma once

#include <Arduino.h>

void initWebClient();
void queueTelemetrySend(const String& payload);