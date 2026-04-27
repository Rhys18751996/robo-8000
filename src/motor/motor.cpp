// src/motor/motor.cpp
#include "motor.h"
#include <Arduino.h>

// ---- CONFIG (change later for DRV8833) ----
namespace {
    // LEFT motor
    const int L_IN1 = 18;
    const int L_IN2 = 19;

    // RIGHT motor
    const int R_IN1 = 21;
    const int R_IN2 = 22;

    const int PWM_FREQ = 20000;
    const int PWM_RES = 8; // 0-255
}

// ---- INIT ----
void initMotors() {
    pinMode(L_IN1, OUTPUT);
    pinMode(L_IN2, OUTPUT);
    pinMode(R_IN1, OUTPUT);
    pinMode(R_IN2, OUTPUT);

    ledcAttach(L_IN1, PWM_FREQ, PWM_RES);
    ledcAttach(L_IN2, PWM_FREQ, PWM_RES);
    ledcAttach(R_IN1, PWM_FREQ, PWM_RES);
    ledcAttach(R_IN2, PWM_FREQ, PWM_RES);
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
        drive(L_IN1, L_IN2, speed);
    } else {
        drive(R_IN1, R_IN2, speed);
    }
}

// ---- HELPERS ----
void stopMotors() {
    setMotor(LEFT, 0);
    setMotor(RIGHT, 0);
}

void brakeMotors() {
    // both HIGH = brake for most H-bridges
    ledcWrite(L_IN1, 255);
    ledcWrite(L_IN2, 255);
    ledcWrite(R_IN1, 255);
    ledcWrite(R_IN2, 255);
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