// src/utils/log.h

#pragma once

enum LogLevel {
    INFO,
    WARN,
    ERROR
};

void log(LogLevel level, const char* message);
void logf(LogLevel level, const char* format, ...);