#pragma once

#include <Bluepad32.h>

class GamepadManager {
public:
    static void setup();
    static void update();

private:
    static ControllerPtr controllers[BP32_MAX_GAMEPADS];

    static void onConnectedController(ControllerPtr ctl);
    static void onDisconnectedController(ControllerPtr ctl);

    static void processGamepad(ControllerPtr ctl);
};