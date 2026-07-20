#include "AudioEngine.h"

#include <algorithm>
#include <cmath>

#include "Logger.h"
#include "ProjectConfig.h"

namespace vibhearing {
namespace {

constexpr float kTargetRms = 0.08F;
constexpr float kMinimumGain = 0.5F;
constexpr float kMaximumGain = 24.0F;
constexpr float kGainSmoothing = 0.05F;
constexpr float kNoiseRiseRate = 0.002F;
constexpr float kNoiseFallRate = 0.04F;
constexpr float kMinimumNoiseFloor = 0.00005F;

}  // namespace

bool AudioEngine::begin() {
  i2s_chan_config_t channelConfig =
      I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_AUTO, I2S_ROLE_MASTER);
  channelConfig.dma_desc_num = 8;
  channelConfig.dma_frame_num = config::kAudioFrameSamples;
  i2s_std_config_t standardConfig = {
      .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(config::kSampleRateHz),
      .slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(
          I2S_DATA_BIT_WIDTH_32BIT, I2S_SLOT_MODE_MONO),
      .gpio_cfg =
          {
              .mclk = I2S_GPIO_UNUSED,
              .bclk = config::kI2sBclkPin,
              .ws = config::kI2sWsPin,
              .dout = I2S_GPIO_UNUSED,
              .din = config::kI2sDataPin,
              .invert_flags =
                  {
                      .mclk_inv = false,
                      .bclk_inv = false,
                      .ws_inv = false,
                  },
          },
  };

  esp_err_t result =
      i2s_new_channel(&channelConfig, nullptr, &receiveChannel_);
  if (result != ESP_OK) {
    Logger::error("I2S channel allocation failed", result);
    return false;
  }
  result = i2s_channel_init_std_mode(receiveChannel_, &standardConfig);
  if (result != ESP_OK) {
    Logger::error("I2S standard mode initialization failed", result);
    i2s_del_channel(receiveChannel_);
    receiveChannel_ = nullptr;
    return false;
  }
  result = i2s_channel_enable(receiveChannel_);
  if (result != ESP_OK) {
    Logger::error("I2S receive channel enable failed", result);
    i2s_del_channel(receiveChannel_);
    receiveChannel_ = nullptr;
    return false;
  }
  Logger::info("I2S microphone ready");
  return true;
}

bool AudioEngine::capture(AudioFeatures& features) {
  size_t bytesRead = 0;
  const esp_err_t result = i2s_channel_read(
      receiveChannel_, rawSamples_.data(), sizeof(rawSamples_), &bytesRead, 100);
  if (result != ESP_OK || bytesRead == 0U) {
    Logger::error("I2S read failed", result);
    return false;
  }

  buffer_.load(rawSamples_.data(), bytesRead / sizeof(rawSamples_[0]));
  extractLevelFeatures(features);
  updateAdaptiveLevels(features);
  features.bands = fft_.analyze(buffer_.samples(), gain_);
  features.gain = gain_;
  features.noiseFloor = noiseFloor_;
  return true;
}

void AudioEngine::extractLevelFeatures(AudioFeatures& features) {
  float sumSquares = 0.0F;
  float peak = 0.0F;
  for (size_t index = 0; index < buffer_.size(); ++index) {
    const float sample = buffer_.samples()[index];
    sumSquares += sample * sample;
    peak = std::max(peak, std::abs(sample));
  }
  const float divisor = std::max<size_t>(buffer_.size(), 1U);
  features.rms = std::sqrt(sumSquares / divisor);
  features.peak = peak;

  constexpr size_t kWaveformStride =
      config::kAudioFrameSamples / config::kWaveformSamples;
  static_assert(kWaveformStride > 0U,
                "Waveform sample count cannot exceed the audio frame");
  for (size_t index = 0; index < features.waveform.size(); ++index) {
    const float sample = std::clamp(
        buffer_.samples()[index * kWaveformStride], -1.0F, 1.0F);
    features.waveform[index] =
        static_cast<int16_t>(sample * static_cast<float>(INT16_MAX));
  }
}

void AudioEngine::updateAdaptiveLevels(AudioFeatures& features) {
  const float desiredGain =
      std::clamp(kTargetRms / std::max(features.rms, kMinimumNoiseFloor),
                 kMinimumGain, kMaximumGain);
  gain_ += (desiredGain - gain_) * kGainSmoothing;

  const float noiseRate =
      features.rms < noiseFloor_ ? kNoiseFallRate : kNoiseRiseRate;
  noiseFloor_ += (features.rms - noiseFloor_) * noiseRate;
  noiseFloor_ = std::max(noiseFloor_, kMinimumNoiseFloor);
}

}  // namespace vibhearing
