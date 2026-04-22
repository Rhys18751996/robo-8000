#pragma once

#include "types.h"

void initModeManager();
void updateModeManager();

Mode getSystemMode();
void setSystemMode(Mode mode);
void toggleSystemMode();
void logSystemMode();

bool isRunMode();
