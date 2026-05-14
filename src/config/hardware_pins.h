#pragma once

namespace HardwarePins {

// Motor driver GPIO assignments.
constexpr int kLeftIn1 = 18;
constexpr int kLeftIn2 = 19;
constexpr int kRightIn1 = 21;
constexpr int kRightIn2 = 22;

// Shared motor PWM setup.
constexpr int kMotorPwmFreq = 20000;
constexpr int kMotorPwmResolutionBits = 8;  // 0-255

}  // namespace HardwarePins
