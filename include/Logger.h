#pragma once

#include <Arduino.h>

namespace vibhearing {

class Logger final {
 public:
  static void begin(uint32_t baudRate);
  static void info(const char* message);
  static void error(const char* message, esp_err_t error);
};

}  // namespace vibhearing
