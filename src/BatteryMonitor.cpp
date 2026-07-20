#include "BatteryMonitor.h"

namespace vibhearing {

bool BatteryMonitor::begin() {
  // A divider and ADC pin must be defined before voltage measurement is safe.
  return true;
}

BatteryStatus BatteryMonitor::read() const { return {}; }

}  // namespace vibhearing
