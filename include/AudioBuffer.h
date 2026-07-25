#pragma once

#include <array>
#include <cstddef>

#include "SignalConfig.h"

namespace vibhearing {

class AudioBuffer final {
 public:
  using Samples = std::array<float, config::kAudioFrameSamples>;

  void load(const int32_t* rawSamples, size_t rawWordCount,
            size_t sampleStride = 1);
  const Samples& samples() const;
  size_t size() const;

 private:
  Samples samples_{};
  size_t size_{0};
};

}  // namespace vibhearing
