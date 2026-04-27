// src/motor/motor.h
#pragma once

#include <stdint.h>

enum MotorSide {
    LEFT = 0,
    RIGHT = 1
};

// Speed: -255 → +255
void initMotors();
void setMotor(MotorSide side, int speed);

// Helpers
void stopMotors();
void brakeMotors();
void setTank(int left, int right);
void setArcade(float linear, float angular);