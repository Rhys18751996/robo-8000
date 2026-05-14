// src/motor/motor.cpp
#include "motor.h"
#include <Arduino.h>

#include "../config/hardware_pins.h"

// ---- CONFIG ----
namespace {
using namespace HardwarePins;
}

// ---- INIT ----
void initMotors() {
    pinMode(kLeftIn1, OUTPUT);
    pinMode(kLeftIn2, OUTPUT);
    pinMode(kRightIn1, OUTPUT);
    pinMode(kRightIn2, OUTPUT);

    ledcAttach(kLeftIn1, kMotorPwmFreq, kMotorPwmResolutionBits);
    ledcAttach(kLeftIn2, kMotorPwmFreq, kMotorPwmResolutionBits);
    ledcAttach(kRightIn1, kMotorPwmFreq, kMotorPwmResolutionBits);
    ledcAttach(kRightIn2, kMotorPwmFreq, kMotorPwmResolutionBits);
}

// ---- CORE ----
static void drive(int pin1, int pin2, int speed) {
    speed = constrain(speed, -255, 255);

    if (speed > 0) {
        ledcWrite(pin1, speed);
        ledcWrite(pin2, 0);
    } else if (speed < 0) {
        ledcWrite(pin1, 0);
        ledcWrite(pin2, -speed);
    } else {
        ledcWrite(pin1, 0);
        ledcWrite(pin2, 0);
    }
}

void setMotor(MotorSide side, int speed) {
    if (side == LEFT) {
        drive(kLeftIn1, kLeftIn2, speed);
    } else {
        drive(kRightIn1, kRightIn2, speed);
    }
}

// ---- HELPERS ----
void stopMotors() {
    setMotor(LEFT, 0);
    setMotor(RIGHT, 0);
}

void brakeMotors() {
    // both HIGH = brake for most H-bridges
    ledcWrite(kLeftIn1, 255);
    ledcWrite(kLeftIn2, 255);
    ledcWrite(kRightIn1, 255);
    ledcWrite(kRightIn2, 255);
}

void setTank(int left, int right) {
    setMotor(LEFT, left);
    setMotor(RIGHT, right);
}

// linear: forward/back (-1 → 1)
// angular: turn (-1 → 1)
void setArcade(float linear, float angular) {
    float left = linear + angular;
    float right = linear - angular;

    // normalize
    float maxMag = max(abs(left), abs(right));
    if (maxMag > 1.0f) {
        left /= maxMag;
        right /= maxMag;
    }

    setTank(left * 255, right * 255);
}