#pragma once

#include "AudioEvent.h"

namespace vibhearing {

inline const char* audioEventName(const AudioEventType type) {
  switch (type) {
    case AudioEventType::HumanVoice:
      return "human_voice";
    case AudioEventType::AnimalVocalization:
      return "animal_vocalization_candidate";
    case AudioEventType::ImpulsiveSound:
      return "impulsive_sound";
    case AudioEventType::TonalAlert:
      return "tonal_alert";
    case AudioEventType::ContinuousNoise:
      return "continuous_noise";
    case AudioEventType::EnvironmentalSound:
      return "environmental_sound";
    case AudioEventType::None:
      return "none";
    default:
      return "unimplemented";
  }
}

}  // namespace vibhearing
