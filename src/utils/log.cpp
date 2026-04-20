// src/utils/log.cpp

#include <Arduino.h>
#include "log.h"
#include <stdarg.h>

void log(LogLevel level, const char* message) {
    const char* prefix;

    switch (level) {
        case INFO: prefix = "[INFO]"; break;
        case WARN: prefix = "[WARN]"; break;
        case ERROR: prefix = "[ERROR]"; break;
        default: prefix = "[LOG]"; break;
    }

    Serial.print(prefix);
    Serial.print(" ");
    Serial.println(message);
}

/// @brief usage: logf(INFO, "Text: %lu", myVariable);
/// @param level 
/// @param format 
/// @param  
void logf(LogLevel level, const char* format, ...) {
    const char* prefix;

    switch (level) {
        case INFO: prefix = "[INFO]"; break;
        case WARN: prefix = "[WARN]"; break;
        case ERROR: prefix = "[ERROR]"; break;
        default: prefix = "[LOG]"; break;
    }

    char buffer[150];

    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    Serial.print(prefix);
    Serial.print(" ");
    Serial.println(buffer);
}