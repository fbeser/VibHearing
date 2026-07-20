#pragma once

#include "AudioEngine.h"
#include "AudioEvent.h"

namespace vibhearing {

class AudioEventDetector {
 public:
  virtual ~AudioEventDetector() = default;
  virtual AudioEventType eventType() const = 0;
  virtual float detect(const AudioFeatures& features) = 0;
};

}  // namespace vibhearing
