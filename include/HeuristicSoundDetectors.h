#pragma once

#include "AudioEventDetector.h"

namespace vibhearing {

class AnimalVocalizationDetector final : public AudioEventDetector {
 public:
  AudioEventType eventType() const override;
  float detect(const AudioFeatures& features) override;

 private:
  uint8_t activeFrames_{0};
};

class ImpulsiveSoundDetector final : public AudioEventDetector {
 public:
  AudioEventType eventType() const override;
  float detect(const AudioFeatures& features) override;
};

class TonalAlertDetector final : public AudioEventDetector {
 public:
  AudioEventType eventType() const override;
  float detect(const AudioFeatures& features) override;

 private:
  uint8_t activeFrames_{0};
};

class ContinuousNoiseDetector final : public AudioEventDetector {
 public:
  AudioEventType eventType() const override;
  float detect(const AudioFeatures& features) override;

 private:
  uint8_t activeFrames_{0};
  float previousRms_{0.0F};
};

class EnvironmentalSoundDetector final : public AudioEventDetector {
 public:
  AudioEventType eventType() const override;
  float detect(const AudioFeatures& features) override;

 private:
  uint8_t activeFrames_{0};
};

}  // namespace vibhearing
