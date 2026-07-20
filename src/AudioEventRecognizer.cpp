#include "AudioEventRecognizer.h"

#include <algorithm>
#include <cmath>

namespace vibhearing {
namespace {

constexpr float kMinimumStrengthDb = 4.0F;
constexpr float kMaximumStrengthDb = 20.0F;
constexpr float kDecibelsPerAmplitudeRatio = 20.0F;

float calculateAcousticStrength(const AudioFeatures& features) {
  const float signalToNoise =
      features.rms / std::max(features.noiseFloor, 0.000001F);
  const float decibels =
      kDecibelsPerAmplitudeRatio * std::log10(std::max(signalToNoise, 1.0F));
  return std::clamp((decibels - kMinimumStrengthDb) /
                        (kMaximumStrengthDb - kMinimumStrengthDb),
                    0.0F, 1.0F);
}

}  // namespace

bool AudioEventRecognizer::addDetector(AudioEventDetector& detector) {
  if (detectorCount_ >= detectors_.size()) {
    return false;
  }
  detectors_[detectorCount_++] = &detector;
  return true;
}

AudioEvent AudioEventRecognizer::recognize(const AudioFeatures& features,
                                           const uint32_t timestampMs) {
  AudioEvent bestEvent{};
  bestEvent.timestampMs = timestampMs;
  for (size_t index = 0; index < detectorCount_; ++index) {
    const float confidence = detectors_[index]->detect(features);
    if (confidence > bestEvent.confidence) {
      bestEvent.type = detectors_[index]->eventType();
      bestEvent.confidence = confidence;
    }
  }
  if (bestEvent.type != AudioEventType::None) {
    bestEvent.acousticStrength = calculateAcousticStrength(features);
  }
  return bestEvent;
}

}  // namespace vibhearing
