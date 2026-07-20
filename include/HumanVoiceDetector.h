#pragma once

#include "AudioEventDetector.h"
#include "RuntimeSettings.h"

namespace vibhearing {

class HumanVoiceDetector final : public AudioEventDetector {
 public:
  explicit HumanVoiceDetector(const RuntimeSettings& settings);
  AudioEventType eventType() const override;
  float detect(const AudioFeatures& features) override;

 private:
  const RuntimeSettings& settings_;
  uint8_t activeFrames_{0};
  uint8_t inactiveFrames_{0};
  float previousRms_{0.0F};
  BandEnergies previousBands_{};
  float temporalVariation_{0.0F};
  bool hasPreviousFrame_{false};
};

}  // namespace vibhearing
