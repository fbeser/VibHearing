#pragma once

namespace vibhearing {

struct BatteryStatus {
  float voltage{0.0F};
  float percentage{0.0F};
  bool available{false};
};

class BatteryMonitor final {
 public:
  bool begin();
  BatteryStatus read() const;
};

}  // namespace vibhearing
