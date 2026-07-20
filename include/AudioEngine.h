#pragma once

#include <array>

#include <driver/i2s_std.h>

#include "AudioBuffer.h"
#include "FFTAnalyzer.h"

namespace vibhearing {

struct AudioFeatures {
  float rms{0.0F};
  float peak{0.0F};
  float gain{1.0F};
  float noiseFloor{0.001F};
  BandEnergies bands{};
  std::array<int16_t, config::kWaveformSamples> waveform{};
};

class AudioEngine final {
 public:
  bool begin();
  bool capture(AudioFeatures& features);

 private:
  void extractLevelFeatures(AudioFeatures& features);
  void updateAdaptiveLevels(AudioFeatures& features);

  std::array<int32_t, config::kAudioFrameSamples> rawSamples_{};
  AudioBuffer buffer_{};
  FFTAnalyzer fft_{};
  i2s_chan_handle_t receiveChannel_{nullptr};
  float gain_{1.0F};
  float noiseFloor_{0.001F};
};

}  // namespace vibhearing
