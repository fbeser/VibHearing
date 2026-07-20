#include "FFTAnalyzer.h"

#include <algorithm>
#include <cmath>

namespace vibhearing {
namespace {

constexpr float kPi = 3.14159265358979323846F;
constexpr std::array<float, 7> kBandEdgesHz{60.0F, 250.0F, 500.0F,
                                            1000.0F, 2000.0F, 4000.0F,
                                            8000.0F};
constexpr size_t kFftBits = 8;
static_assert((1U << kFftBits) == config::kAudioFrameSamples,
              "FFT frame size must be a power of two");

}  // namespace

BandEnergies FFTAnalyzer::analyze(const AudioBuffer::Samples& samples,
                                  const float gain) {
  for (size_t index = 0; index < samples.size(); ++index) {
    const float window = 0.5F -
                         0.5F * std::cos((2.0F * kPi * index) /
                                         (samples.size() - 1U));
    frame_[index] = std::complex<float>(samples[index] * gain * window, 0.0F);
  }

  transform(frame_);
  BandEnergies energies{};
  std::array<size_t, energies.size()> binCounts{};

  for (size_t bin = 1; bin <= frame_.size() / 2U; ++bin) {
    const float frequency =
        static_cast<float>(bin * config::kSampleRateHz) / frame_.size();
    for (size_t band = 0; band < energies.size(); ++band) {
      const bool isLastBand = band == energies.size() - 1U;
      if (frequency >= kBandEdgesHz[band] &&
          (frequency < kBandEdgesHz[band + 1U] ||
           (isLastBand && frequency <= kBandEdgesHz[band + 1U]))) {
        energies[band] += std::norm(frame_[bin]);
        ++binCounts[band];
        break;
      }
    }
  }

  for (size_t band = 0; band < energies.size(); ++band) {
    if (binCounts[band] > 0U) {
      energies[band] =
          std::sqrt(energies[band] / static_cast<float>(binCounts[band]));
    }
  }
  return energies;
}

void FFTAnalyzer::transform(ComplexFrame& frame) const {
  for (size_t index = 0; index < frame.size(); ++index) {
    const size_t reversed = reverseBits(index);
    if (reversed > index) {
      std::swap(frame[index], frame[reversed]);
    }
  }

  for (size_t length = 2; length <= frame.size(); length <<= 1U) {
    const float angle = -2.0F * kPi / static_cast<float>(length);
    const std::complex<float> step(std::cos(angle), std::sin(angle));
    for (size_t offset = 0; offset < frame.size(); offset += length) {
      std::complex<float> twiddle(1.0F, 0.0F);
      for (size_t index = 0; index < length / 2U; ++index) {
        const auto even = frame[offset + index];
        const auto odd = frame[offset + index + length / 2U] * twiddle;
        frame[offset + index] = even + odd;
        frame[offset + index + length / 2U] = even - odd;
        twiddle *= step;
      }
    }
  }
}

size_t FFTAnalyzer::reverseBits(size_t value) {
  size_t reversed = 0;
  for (size_t bit = 0; bit < kFftBits; ++bit) {
    reversed = (reversed << 1U) | (value & 1U);
    value >>= 1U;
  }
  return reversed;
}

}  // namespace vibhearing
