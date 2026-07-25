#pragma once

#include <array>

#include <driver/i2s_std.h>

#include "AudioBuffer.h"
#include "AudioFeatures.h"
#include "FFTAnalyzer.h"
#include "ProjectConfig.h"

namespace vibhearing {

class AudioEngine final {
 public:
  bool begin();
  bool capture(AudioFeatures& features);

 private:
  void extractLevelFeatures(AudioFeatures& features);
  void updateAdaptiveLevels(AudioFeatures& features);

  std::array<int32_t,
             config::kAudioFrameSamples * config::kI2sSlotsPerFrame>
      rawSamples_{};
  AudioBuffer buffer_{};
  FFTAnalyzer fft_{};
  i2s_chan_handle_t receiveChannel_{nullptr};
  float gain_{1.0F};
  float noiseFloor_{0.001F};
};

}  // namespace vibhearing
