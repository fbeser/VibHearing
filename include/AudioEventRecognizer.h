#pragma once

#include <array>

#include "AudioEventDetector.h"

namespace vibhearing {

class AudioEventRecognizer final {
 public:
  static constexpr size_t kMaximumDetectors = 8;

  bool addDetector(AudioEventDetector& detector);
  AudioEvent recognize(const AudioFeatures& features, uint32_t timestampMs);

 private:
  std::array<AudioEventDetector*, kMaximumDetectors> detectors_{};
  size_t detectorCount_{0};
};

}  // namespace vibhearing
