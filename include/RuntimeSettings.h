#pragma once

#include <Arduino.h>

#include "ProjectConfig.h"

namespace vibhearing {

struct RuntimeSettings {
  bool motorEnabled{true};
  bool serialMetricsEnabled{true};
  float minimumVoiceSnr{2.00F};
  float minimumVoiceRatio{0.38F};
  float closeVoiceSnr{6.0F};
  float minimumEventConfidence{0.55F};
  uint8_t minimumHapticDuty{config::kMotorMinimumHapticDuty};
  uint8_t maximumHapticDuty{config::kMotorSafeMaximumDuty};
  uint32_t eventRetriggerGuardMs{450};
};

}  // namespace vibhearing
