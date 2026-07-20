#pragma once

#include <algorithm>
#include <array>

#include "AudioEngine.h"

namespace vibhearing::sound_features {

inline float bandMagnitude(const AudioFeatures& features,
                           const FrequencyBand band) {
  return features.bands[static_cast<size_t>(band)];
}

inline float signalToNoise(const AudioFeatures& features) {
  return features.rms / std::max(features.noiseFloor, 0.000001F);
}

inline float totalBandMagnitude(const AudioFeatures& features) {
  float total = 0.0F;
  for (const float magnitude : features.bands) {
    total += magnitude;
  }
  return total;
}

inline float spectralCentroidHz(const AudioFeatures& features) {
  constexpr std::array<float, 6> kBandCentersHz{155.0F, 375.0F, 750.0F,
                                                1500.0F, 3000.0F, 6000.0F};
  const float total = totalBandMagnitude(features);
  float weightedFrequency = 0.0F;
  for (size_t index = 0; index < features.bands.size(); ++index) {
    weightedFrequency += features.bands[index] * kBandCentersHz[index];
  }
  return weightedFrequency / std::max(total, 0.000001F);
}

inline float crestFactor(const AudioFeatures& features) {
  return features.peak / std::max(features.rms, 0.000001F);
}

}  // namespace vibhearing::sound_features
