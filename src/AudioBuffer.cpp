#include "AudioBuffer.h"

#include <algorithm>

namespace vibhearing {

	void AudioBuffer::load(const int32_t* rawSamples, const size_t rawWordCount, const size_t sampleStride) {
		const size_t stride = std::max<size_t>(sampleStride, 1U);
		size_ = std::min(rawWordCount / stride, samples_.size());
		float frameMean = 0.0F;
		for (size_t index = 0; index < size_; ++index) {
			const int32_t sample = rawSamples[index * stride] >> config::kMicrophoneWordShift;
			samples_[index] = static_cast<float>(sample) / config::kMicrophoneFullScale;
			frameMean += samples_[index];
		}
		if (size_ > 0U) {
			frameMean /= static_cast<float>(size_);
			for (size_t index = 0; index < size_; ++index) {
				samples_[index] -= frameMean;
			}
		}
		std::fill(samples_.begin() + size_, samples_.end(), 0.0F);
	}

	const AudioBuffer::Samples& AudioBuffer::samples() const {
		return samples_;
	}

	size_t AudioBuffer::size() const {
		return size_;
	}

}  // namespace vibhearing
