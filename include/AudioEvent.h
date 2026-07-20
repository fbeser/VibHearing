#pragma once

#include <cstdint>

namespace vibhearing {

enum class AudioEventType : uint8_t {
  None,
  HumanVoice,
  CatMeow,
  DogBark,
  DoorBell,
  Alarm,
  Clap,
  Keyboard,
  VacuumCleaner,
  CarHorn,
  GlassBreak,
  AnimalVocalization,
  ImpulsiveSound,
  TonalAlert,
  ContinuousNoise,
  EnvironmentalSound
};

struct AudioEvent {
  AudioEventType type{AudioEventType::None};
  float confidence{0.0F};
  float acousticStrength{0.0F};
  uint32_t timestampMs{0};
};

}  // namespace vibhearing
