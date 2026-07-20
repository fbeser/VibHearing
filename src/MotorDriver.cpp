#include "MotorDriver.h"

#include "ProjectConfig.h"

namespace vibhearing {

MotorDriver::MotorDriver(const uint8_t pin, const bool enabled)
    : pin_(pin), enabled_(enabled) {}

bool MotorDriver::begin() {
  if (!enabled_) {
    return true;
  }
  const bool attached =
      ledcAttach(pin_, config::kMotorPwmFrequencyHz,
                 config::kMotorPwmResolutionBits);
  stop();
  return attached;
}

void MotorDriver::setIntensity(const uint8_t intensity) {
  if (enabled_) {
    ledcWrite(pin_, intensity);
  }
}

void MotorDriver::stop() { setIntensity(0); }

}  // namespace vibhearing
