#include <cmath>

#include <unity.h>

#include "AudioBuffer.h"
#include "AudioFeatures.h"
#include "FFTAnalyzer.h"
#include "SoundFeatureMath.h"

void setUp() {
}
void tearDown() {
}

namespace {

	constexpr float kPi = 3.14159265358979323846F;

	vibhearing::AudioBuffer::Samples makeTone(const float frequencyHz) {
		vibhearing::AudioBuffer::Samples samples{};
		for (size_t index = 0; index < samples.size(); ++index) {
			samples[index] = std::sin(2.0F * kPi * frequencyHz * static_cast<float>(index) /
									  static_cast<float>(vibhearing::config::kSampleRateHz));
		}
		return samples;
	}

	void assertDominantBand(const float frequencyHz, const vibhearing::FrequencyBand expectedBand) {
		vibhearing::FFTAnalyzer analyzer;
		const auto energies = analyzer.analyze(makeTone(frequencyHz), 1.0F);
		const size_t expectedIndex = static_cast<size_t>(expectedBand);
		for (size_t index = 0; index < energies.size(); ++index) {
			if (index != expectedIndex) {
				TEST_ASSERT_GREATER_THAN(energies[index], energies[expectedIndex]);
			}
		}
	}

	void test_generated_tones_land_in_expected_bands() {
		assertDominantBand(125.0F, vibhearing::FrequencyBand::Bass);
		assertDominantBand(375.0F, vibhearing::FrequencyBand::LowMid);
		assertDominantBand(750.0F, vibhearing::FrequencyBand::Mid);
		assertDominantBand(1500.0F, vibhearing::FrequencyBand::Voice);
		assertDominantBand(3000.0F, vibhearing::FrequencyBand::Presence);
		assertDominantBand(6000.0F, vibhearing::FrequencyBand::High);
	}

	void test_tones_at_band_edges_use_the_upper_band() {
		assertDominantBand(250.0F, vibhearing::FrequencyBand::LowMid);
		assertDominantBand(500.0F, vibhearing::FrequencyBand::Mid);
		assertDominantBand(1000.0F, vibhearing::FrequencyBand::Voice);
		assertDominantBand(2000.0F, vibhearing::FrequencyBand::Presence);
		assertDominantBand(4000.0F, vibhearing::FrequencyBand::High);
	}

	void test_fft_gain_scales_every_band_magnitude() {
		vibhearing::FFTAnalyzer analyzer;
		const auto samples = makeTone(1500.0F);
		const auto unityGain = analyzer.analyze(samples, 1.0F);
		const auto doubleGain = analyzer.analyze(samples, 2.0F);

		for (size_t index = 0; index < unityGain.size(); ++index) {
			TEST_ASSERT_FLOAT_WITHIN(0.0001F, unityGain[index] * 2.0F, doubleGain[index]);
		}
	}

	void test_silent_frame_has_zero_band_energy() {
		vibhearing::FFTAnalyzer analyzer;
		const vibhearing::AudioBuffer::Samples silence{};
		const auto energies = analyzer.analyze(silence, 1.0F);
		for (const float energy : energies) {
			TEST_ASSERT_FLOAT_WITHIN(0.000001F, 0.0F, energy);
		}
	}

	void test_feature_math_handles_zero_floor_and_empty_spectrum() {
		vibhearing::AudioFeatures features{};
		features.rms = 0.5F;
		features.peak = 0.75F;
		features.noiseFloor = 0.0F;

		TEST_ASSERT_FLOAT_WITHIN(1.0F, 500000.0F, vibhearing::sound_features::signalToNoise(features));
		TEST_ASSERT_FLOAT_WITHIN(0.000001F, 0.0F, vibhearing::sound_features::spectralCentroidHz(features));
		TEST_ASSERT_FLOAT_WITHIN(0.000001F, 1.5F, vibhearing::sound_features::crestFactor(features));
	}

	void test_audio_buffer_compacts_the_active_i2s_slot() {
		constexpr int32_t kHalfScale = 1073741824;
		const int32_t interleavedWords[]{
			kHalfScale, 123, 0, 456, -kHalfScale, 789,
		};
		vibhearing::AudioBuffer buffer;

		buffer.load(interleavedWords, 6, 2);

		TEST_ASSERT_EQUAL_UINT32(3, buffer.size());
		TEST_ASSERT_FLOAT_WITHIN(0.000001F, 0.5F, buffer.samples()[0]);
		TEST_ASSERT_FLOAT_WITHIN(0.000001F, 0.0F, buffer.samples()[1]);
		TEST_ASSERT_FLOAT_WITHIN(0.000001F, -0.5F, buffer.samples()[2]);
	}

}  // namespace

int main(int, char**) {
	UNITY_BEGIN();
	RUN_TEST(test_generated_tones_land_in_expected_bands);
	RUN_TEST(test_tones_at_band_edges_use_the_upper_band);
	RUN_TEST(test_fft_gain_scales_every_band_magnitude);
	RUN_TEST(test_silent_frame_has_zero_band_energy);
	RUN_TEST(test_feature_math_handles_zero_floor_and_empty_spectrum);
	RUN_TEST(test_audio_buffer_compacts_the_active_i2s_slot);
	return UNITY_END();
}
