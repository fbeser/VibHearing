#pragma once

#include <array>
#include <complex>

#include "AudioBuffer.h"

namespace vibhearing {

enum class FrequencyBand : uint8_t {
  Bass,
  LowMid,
  Mid,
  Voice,
  Presence,
  High,
  Count
};

using BandEnergies =
    std::array<float, static_cast<size_t>(FrequencyBand::Count)>;

class FFTAnalyzer final {
 public:
  BandEnergies analyze(const AudioBuffer::Samples& samples, float gain);

 private:
  using ComplexFrame =
      std::array<std::complex<float>, config::kAudioFrameSamples>;

  void transform(ComplexFrame& frame) const;
  static size_t reverseBits(size_t value);

  ComplexFrame frame_{};
};

}  // namespace vibhearing
