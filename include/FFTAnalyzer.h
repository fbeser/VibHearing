#pragma once

#include <array>
#include <complex>

#include "AudioBuffer.h"
#include "AudioFeatures.h"

namespace vibhearing {

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
