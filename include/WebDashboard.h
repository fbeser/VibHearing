#pragma once

#include <WebServer.h>

#include "AudioEngine.h"
#include "AudioEvent.h"
#include "RuntimeSettings.h"

namespace vibhearing {

class WebDashboard final {
 public:
  explicit WebDashboard(RuntimeSettings& settings);

  void begin();
  void update();
  void updateAudio(const AudioFeatures& features, const AudioEvent& event);
  bool connected() const;

 private:
  void registerRoutes();
  void startServerIfReady();
  void reconnectIfNeeded();
  void sendDashboard();
  void sendStatus();
  void sendSettings();
  void applySettings();
  String statusJson() const;
  String settingsJson() const;

  RuntimeSettings& settings_;
  WebServer server_{80};
  AudioFeatures latestFeatures_{};
  AudioEvent latestEvent_{};
  bool hasAudio_{false};
  bool serverStarted_{false};
  uint32_t connectionAttemptMs_{0};
};

}  // namespace vibhearing
