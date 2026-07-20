#pragma once

#include <Arduino.h>

namespace vibhearing {

class MotorDriver final {
 public:
  MotorDriver(uint8_t pin, bool enabled);

  bool begin();
  void setIntensity(uint8_t intensity);
  void stop();

 private:
  uint8_t pin_;
  bool enabled_;
};

}  // namespace vibhearing
