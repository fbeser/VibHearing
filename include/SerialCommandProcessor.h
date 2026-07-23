#pragma once

#include <Arduino.h>

namespace vibhearing {

class SerialCommandProcessor final {
 public:
  void update();

 private:
  static constexpr size_t kMaximumCommandLength = 31;
  static constexpr uint32_t kCommandIdleTimeoutMs = 50;

  void executeCommand();
  void clearCommand();

  char command_[kMaximumCommandLength + 1]{};
  size_t commandLength_{0};
  uint32_t lastByteReceivedMs_{0};
};

}  // namespace vibhearing
