#include "HumanVoiceDetector.h"

#include <algorithm>
#include <cmath>

namespace vibhearing {
namespace {

constexpr float kCloseVoiceMinimumRatio = 0.22F;
constexpr float kHighBandVoiceWeight = 0.65F;
constexpr uint8_t kAttackFrames = 5;
constexpr uint8_t kReleaseFrames = 10;
constexpr float kVariationSmoothing = 0.18F;
constexpr float kMinimumTemporalVariation = 0.06F;
constexpr float kCloseVoiceMinimumVariation = 0.035F;

}  // namespace

HumanVoiceDetector::HumanVoiceDetector(const RuntimeSettings& settings)
    : settings_(settings) {}

AudioEventType HumanVoiceDetector::eventType() const {
  return AudioEventType::HumanVoice;
}

float HumanVoiceDetector::detect(const AudioFeatures& features) {
  const float bassEnergy =
      features.bands[static_cast<size_t>(FrequencyBand::Bass)];
  const float highEnergy =
      features.bands[static_cast<size_t>(FrequencyBand::High)];
  const float voiceEnergy =
      features.bands[static_cast<size_t>(FrequencyBand::LowMid)] +
      features.bands[static_cast<size_t>(FrequencyBand::Mid)] +
      features.bands[static_cast<size_t>(FrequencyBand::Voice)] +
      features.bands[static_cast<size_t>(FrequencyBand::Presence)] +
      highEnergy * kHighBandVoiceWeight;
  const float totalEnergy = bassEnergy + voiceEnergy +
                            highEnergy * (1.0F - kHighBandVoiceWeight);
  const float voiceRatio = voiceEnergy / std::max(totalEnergy, 0.000001F);
  const float signalToNoise =
      features.rms / std::max(features.noiseFloor, 0.000001F);
  float instantVariation = 0.0F;
  if (hasPreviousFrame_) {
    const float rmsChange =
        std::abs(features.rms - previousRms_) /
        std::max({features.rms, previousRms_, 0.000001F});
    float bandChange = 0.0F;
    float bandMagnitude = 0.0F;
    for (size_t index = 0; index < features.bands.size(); ++index) {
      bandChange += std::abs(features.bands[index] - previousBands_[index]);
      bandMagnitude += features.bands[index] + previousBands_[index];
    }
    const float spectralChange =
        bandChange / std::max(bandMagnitude, 0.000001F);
    instantVariation =
        std::clamp(0.65F * rmsChange + 0.35F * spectralChange, 0.0F, 1.0F);
  }
  previousRms_ = features.rms;
  previousBands_ = features.bands;
  hasPreviousFrame_ = true;
  temporalVariation_ +=
      (instantVariation - temporalVariation_) * kVariationSmoothing;

  const bool normalVoice = signalToNoise >= settings_.minimumVoiceSnr &&
                           voiceRatio >= settings_.minimumVoiceRatio &&
                           temporalVariation_ >= kMinimumTemporalVariation;
  const bool closeOrClippedVoice = signalToNoise >= settings_.closeVoiceSnr &&
                                   voiceRatio >= kCloseVoiceMinimumRatio &&
                                   temporalVariation_ >=
                                       kCloseVoiceMinimumVariation;
  const bool candidate = normalVoice || closeOrClippedVoice;

  if (candidate) {
    activeFrames_ = std::min<uint8_t>(activeFrames_ + 1U, kAttackFrames);
    inactiveFrames_ = 0;
  } else if (activeFrames_ < kAttackFrames) {
    // Attack frames must be consecutive; intermittent machinery must not
    // accumulate unrelated frames until they appear to be speech.
    activeFrames_ = 0;
    inactiveFrames_ = 0;
  } else {
    inactiveFrames_ = std::min<uint8_t>(inactiveFrames_ + 1U, kReleaseFrames);
    if (inactiveFrames_ >= kReleaseFrames) {
      activeFrames_ = 0;
    }
  }

  const bool active = activeFrames_ >= kAttackFrames &&
                      inactiveFrames_ < kReleaseFrames;
  if (!active) {
    return 0.0F;
  }
  const float ratioScore =
      std::clamp((voiceRatio - settings_.minimumVoiceRatio) /
                     (1.0F - settings_.minimumVoiceRatio),
                 0.0F, 1.0F);
  const float noiseScore =
      std::clamp((signalToNoise - settings_.minimumVoiceSnr) / 3.0F, 0.0F,
                 1.0F);
  return 0.68F + 0.16F * ratioScore + 0.16F * noiseScore;
}

}  // namespace vibhearing
