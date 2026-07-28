#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include "SignalConfig.h"

namespace vibhearing {

	enum class FrequencyBand : uint8_t { Bass, LowMid, Mid, Voice, Presence, High, Count };

	using BandEnergies = std::array<float, static_cast<size_t>(FrequencyBand::Count)>;

	struct AudioFeatures {
		float rms{0.0F};
		float peak{0.0F};
		float gain{1.0F};
		float noiseFloor{0.001F};
		BandEnergies bands{};
		std::array<int16_t, config::kWaveformSamples> waveform{};
	};

}  // namespace vibhearing
