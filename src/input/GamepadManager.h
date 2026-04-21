#pragma once

#include <Bluepad32.h>

#include "input.h"

class GamepadManager {
public:
    static void setup();
    static RawInput read();
    static int readBatteryPercent();

private:
    static ControllerPtr controllers[BP32_MAX_GAMEPADS];

    static void onConnectedController(ControllerPtr ctl);
    static void onDisconnectedController(ControllerPtr ctl);

    static ControllerPtr getActiveController();

    static float clamp01(float v);
    static float clamp11(float v);
    static float normAxis(int v);
    static float normTrigger(int v);
};
