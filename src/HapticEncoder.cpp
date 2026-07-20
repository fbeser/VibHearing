#include "HapticEncoder.h"

#include <algorithm>
#include <cmath>

namespace vibhearing {

HapticEncoder::HapticEncoder(MotorDriver& leftMotor, MotorDriver& rightMotor,
                             const RuntimeSettings& settings)
    : leftMotor_(leftMotor), rightMotor_(rightMotor), settings_(settings) {}

void HapticEncoder::handleEvent(const AudioEvent& event) {
  if (!settings_.motorEnabled || event.type != AudioEventType::HumanVoice ||
      event.confidence < settings_.minimumEventConfidence ||
      (lastEventMs_ != 0U &&
       event.timestampMs - lastEventMs_ < settings_.eventRetriggerGuardMs)) {
    return;
  }
  lastEventMs_ = event.timestampMs;
  startPattern(event.type, event.timestampMs, event.acousticStrength);
}

void HapticEncoder::update(const uint32_t nowMs) {
  if (!active_ || nowMs - stepStartedMs_ < pattern_[stepIndex_].durationMs) {
    return;
  }
  ++stepIndex_;
  if (stepIndex_ >= stepCount_) {
    stop();
    return;
  }
  stepStartedMs_ = nowMs;
  applyCurrentStep();
}

void HapticEncoder::stop() {
  leftMotor_.stop();
  rightMotor_.stop();
  active_ = false;
}

bool HapticEncoder::isActive() const { return active_; }

void HapticEncoder::startPattern(const AudioEventType type,
                                 const uint32_t nowMs,
                                 const float acousticStrength) {
  const uint8_t intensity = intensityForStrength(acousticStrength);
  pattern_ = {};
  switch (type) {
    case AudioEventType::HumanVoice:
      pattern_[0] = {110, intensity, 0};
      pattern_[1] = {70, 0, 0};
      pattern_[2] = {110, intensity, 0};
      stepCount_ = 3;
      break;
    case AudioEventType::AnimalVocalization:
      pattern_[0] = {75, intensity, 0};
      pattern_[1] = {55, 0, 0};
      pattern_[2] = {75, intensity, 0};
      pattern_[3] = {55, 0, 0};
      pattern_[4] = {75, intensity, 0};
      stepCount_ = 5;
      break;
    case AudioEventType::ImpulsiveSound:
      pattern_[0] = {90, intensity, 0};
      stepCount_ = 1;
      break;
    case AudioEventType::TonalAlert:
      pattern_[0] = {220, intensity, 0};
      pattern_[1] = {80, 0, 0};
      pattern_[2] = {80, intensity, 0};
      stepCount_ = 3;
      break;
    case AudioEventType::ContinuousNoise:
      pattern_[0] = {150, intensity, 0};
      pattern_[1] = {120, 0, 0};
      pattern_[2] = {150, intensity, 0};
      stepCount_ = 3;
      break;
    case AudioEventType::EnvironmentalSound:
    default:
      pattern_[0] = {100, intensity, 0};
      stepCount_ = 1;
      break;
  }
  stepIndex_ = 0;
  stepStartedMs_ = nowMs;
  active_ = true;
  applyCurrentStep();
}

uint8_t HapticEncoder::intensityForStrength(
    const float acousticStrength) const {
  const float boundedStrength = std::clamp(acousticStrength, 0.0F, 1.0F);
  const float dutyRange = static_cast<float>(
      settings_.maximumHapticDuty - settings_.minimumHapticDuty);
  return static_cast<uint8_t>(std::lround(
      settings_.minimumHapticDuty + boundedStrength * dutyRange));
}

void HapticEncoder::applyCurrentStep() {
  leftMotor_.setIntensity(pattern_[stepIndex_].leftIntensity);
  rightMotor_.setIntensity(pattern_[stepIndex_].rightIntensity);
}

}  // namespace vibhearing
