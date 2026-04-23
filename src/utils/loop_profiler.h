// src/utils/loop_profiler.h
#pragma once

// Starts timing a loop iteration
void beginLoopProfile();

// Ends timing, updates stats, and logs periodically
void endLoopProfile();