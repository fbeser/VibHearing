#pragma once

#include <Arduino.h>

namespace vibhearing::config {

constexpr char kProductName[] = "VibHearing";
constexpr uint32_t kSerialBaud = 115200;

constexpr gpio_num_t kMotorLeftPin = GPIO_NUM_7;
constexpr gpio_num_t kMotorRightPin = GPIO_NUM_21;
constexpr gpio_num_t kI2sBclkPin = GPIO_NUM_4;
constexpr gpio_num_t kI2sWsPin = GPIO_NUM_5;
constexpr gpio_num_t kI2sDataPin = GPIO_NUM_6;
constexpr uint32_t kSampleRateHz = 16000;
constexpr size_t kAudioFrameSamples = 256;
constexpr size_t kWaveformSamples = kAudioFrameSamples;
constexpr uint8_t kMicrophoneWordShift = 8;
constexpr float kMicrophoneFullScale = 8388608.0F;

constexpr uint32_t kMotorPwmFrequencyHz = 1000;
constexpr uint8_t kMotorPwmResolutionBits = 8;
constexpr uint8_t kMotorMaximumDuty = 255;
constexpr uint8_t kMotorMinimumHapticDuty = 80;
constexpr uint8_t kMotorSafeMaximumDuty = 180;

constexpr uint32_t kMetricsLogPeriodMs = 500;
constexpr bool kEnableRightMotor = false;
constexpr bool kEnableBle = false;
constexpr bool kEnableBatteryMonitor = false;
constexpr bool kEnableExperimentalSoundClassifiers = false;

constexpr char kDeviceHostname[] = "vibhearing";
constexpr uint32_t kWifiConnectionTimeoutMs = 15000;

}  // namespace vibhearing::config
