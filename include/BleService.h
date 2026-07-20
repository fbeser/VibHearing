#pragma once

namespace vibhearing {

class BleService final {
 public:
  bool begin();
  void update();
  bool enabled() const;

 private:
  bool enabled_{false};
};

}  // namespace vibhearing
