// src/control/control.h

#pragma once

void update();
void initControl();

void setInputSnapshotLoggingEnabled(bool enabled);
bool isInputSnapshotLoggingEnabled();

void setButtonChangeLoggingEnabled(bool enabled);
bool isButtonChangeLoggingEnabled();

void setIntentLoggingEnabled(bool enabled);
bool isIntentLoggingEnabled();

void setBatteryLoggingEnabled(bool enabled);
bool isBatteryLoggingEnabled();
