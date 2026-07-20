#include "BleService.h"

#include "ProjectConfig.h"

namespace vibhearing {

bool BleService::begin() {
  enabled_ = config::kEnableBle;
  // The transport remains disabled until the BLE protocol is specified.
  return true;
}

void BleService::update() {}

bool BleService::enabled() const { return enabled_; }

}  // namespace vibhearing
