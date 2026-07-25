#pragma once

#include <cstddef>
#include <cstdint>

namespace vibhearing::config {

constexpr uint32_t kSampleRateHz = 16000;
constexpr size_t kAudioFrameSamples = 256;
constexpr size_t kWaveformSamples = kAudioFrameSamples;
constexpr uint8_t kMicrophoneWordShift = 8;
constexpr float kMicrophoneFullScale = 8388608.0F;

}  // namespace vibhearing::config
