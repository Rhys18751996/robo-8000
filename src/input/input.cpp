// src/input/input.cpp

#include "input.h"
#include "bluepad_adapter.h"

void initInput() {
    initGamepad();
}

RawInput readInput() {
    return readGamepad();
}