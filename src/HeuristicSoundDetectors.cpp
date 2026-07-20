#include "HeuristicSoundDetectors.h"

#include <algorithm>
#include <cmath>

#include "SoundFeatureMath.h"

namespace vibhearing {
namespace {

constexpr float kMinimumSoundSnr = 1.60F;

float normalizedScore(const float value, const float minimum,
                      const float maximum) {
  return std::clamp((value - minimum) / (maximum - minimum), 0.0F, 1.0F);
}

}  // namespace

AudioEventType AnimalVocalizationDetector::eventType() const {
  return AudioEventType::AnimalVocalization;
}

float AnimalVocalizationDetector::detect(const AudioFeatures& features) {
  const float snr = sound_features::signalToNoise(features);
  const float total = sound_features::totalBandMagnitude(features);
  const float vocalMagnitude =
      sound_features::bandMagnitude(features, FrequencyBand::Mid) +
      sound_features::bandMagnitude(features, FrequencyBand::Voice) +
      sound_features::bandMagnitude(features, FrequencyBand::Presence);
  const float vocalRatio = vocalMagnitude / std::max(total, 0.000001F);
  const float centroid = sound_features::spectralCentroidHz(features);
  const bool candidate = snr >= 1.9F && vocalRatio >= 0.20F &&
                         centroid >= 650.0F && centroid <= 3200.0F;
  activeFrames_ = candidate ? std::min<uint8_t>(activeFrames_ + 1U, 4U) : 0U;
  if (activeFrames_ < 4U) {
    return 0.0F;
  }
  return 0.82F + 0.10F * normalizedScore(vocalRatio, 0.20F, 0.60F) +
         0.06F * normalizedScore(snr, 1.9F, 8.0F);
}

AudioEventType ImpulsiveSoundDetector::eventType() const {
  return AudioEventType::ImpulsiveSound;
}

float ImpulsiveSoundDetector::detect(const AudioFeatures& features) {
  const float snr = sound_features::signalToNoise(features);
  const float crest = sound_features::crestFactor(features);
  if (snr < 2.2F || crest < 3.6F) {
    return 0.0F;
  }
  return 0.90F + 0.08F * normalizedScore(crest, 3.6F, 7.0F);
}

AudioEventType TonalAlertDetector::eventType() const {
  return AudioEventType::TonalAlert;
}

float TonalAlertDetector::detect(const AudioFeatures& features) {
  const float snr = sound_features::signalToNoise(features);
  const float usableMagnitude =
      sound_features::bandMagnitude(features, FrequencyBand::Mid) +
      sound_features::bandMagnitude(features, FrequencyBand::Voice) +
      sound_features::bandMagnitude(features, FrequencyBand::Presence) +
      sound_features::bandMagnitude(features, FrequencyBand::High);
  const float dominantMagnitude =
      std::max({sound_features::bandMagnitude(features, FrequencyBand::Mid),
                sound_features::bandMagnitude(features, FrequencyBand::Voice),
                sound_features::bandMagnitude(features,
                                               FrequencyBand::Presence),
                sound_features::bandMagnitude(features, FrequencyBand::High)});
  const float concentration =
      dominantMagnitude / std::max(usableMagnitude, 0.000001F);
  const bool candidate = snr >= 2.0F && concentration >= 0.66F &&
                         usableMagnitude >=
                             sound_features::bandMagnitude(
                                 features, FrequencyBand::Bass) *
                                 0.25F;
  activeFrames_ = candidate ? std::min<uint8_t>(activeFrames_ + 1U, 5U) : 0U;
  if (activeFrames_ < 5U) {
    return 0.0F;
  }
  return 0.88F + 0.10F * normalizedScore(concentration, 0.66F, 0.95F);
}

AudioEventType ContinuousNoiseDetector::eventType() const {
  return AudioEventType::ContinuousNoise;
}

float ContinuousNoiseDetector::detect(const AudioFeatures& features) {
  const float snr = sound_features::signalToNoise(features);
  const float relativeChange =
      std::abs(features.rms - previousRms_) / std::max(previousRms_, 0.000001F);
  previousRms_ = features.rms;
  const bool stable = snr >= 1.8F && relativeChange <= 0.45F;
  activeFrames_ = stable ? std::min<uint8_t>(activeFrames_ + 1U, 20U) : 0U;
  if (activeFrames_ < 20U) {
    return 0.0F;
  }
  return 0.86F + 0.10F * normalizedScore(snr, 1.8F, 8.0F);
}

AudioEventType EnvironmentalSoundDetector::eventType() const {
  return AudioEventType::EnvironmentalSound;
}

float EnvironmentalSoundDetector::detect(const AudioFeatures& features) {
  const float snr = sound_features::signalToNoise(features);
  const bool candidate = snr >= kMinimumSoundSnr;
  activeFrames_ = candidate ? std::min<uint8_t>(activeFrames_ + 1U, 2U) : 0U;
  if (activeFrames_ < 2U) {
    return 0.0F;
  }
  return 0.56F + 0.04F * normalizedScore(snr, kMinimumSoundSnr, 8.0F);
}

}  // namespace vibhearing
