#pragma once

#include <array>

#include "AudioEvent.h"
#include "MotorDriver.h"
#include "RuntimeSettings.h"

namespace vibhearing {

struct HapticStep {
  uint16_t durationMs;
  uint8_t leftIntensity;
  uint8_t rightIntensity;
};

class HapticEncoder final {
 public:
  HapticEncoder(MotorDriver& leftMotor, MotorDriver& rightMotor,
                const RuntimeSettings& settings);

  void handleEvent(const AudioEvent& event);
  void update(uint32_t nowMs);
  void stop();
  bool isActive() const;

 private:
  static constexpr size_t kMaximumSteps = 8;
  using Pattern = std::array<HapticStep, kMaximumSteps>;

  void startPattern(AudioEventType type, uint32_t nowMs,
                    float acousticStrength);
  uint8_t intensityForStrength(float acousticStrength) const;
  void applyCurrentStep();

  MotorDriver& leftMotor_;
  MotorDriver& rightMotor_;
  const RuntimeSettings& settings_;
  Pattern pattern_{};
  size_t stepCount_{0};
  size_t stepIndex_{0};
  uint32_t stepStartedMs_{0};
  uint32_t lastEventMs_{0};
  bool active_{false};
};

}  // namespace vibhearing
