#include <Arduino.h>

#include <algorithm>

#include "AudioEngine.h"
#include "AudioEventRecognizer.h"
#include "AudioEventNames.h"
#include "BatteryMonitor.h"
#include "BleService.h"
#include "HapticEncoder.h"
#include "HeuristicSoundDetectors.h"
#include "HumanVoiceDetector.h"
#include "Logger.h"
#include "MotorDriver.h"
#include "ProjectConfig.h"
#include "RuntimeSettings.h"
#include "WebDashboard.h"

namespace {

using namespace vibhearing;

AudioEngine audioEngine;
RuntimeSettings runtimeSettings;
HumanVoiceDetector humanVoiceDetector(runtimeSettings);
AnimalVocalizationDetector animalVocalizationDetector;
ImpulsiveSoundDetector impulsiveSoundDetector;
TonalAlertDetector tonalAlertDetector;
ContinuousNoiseDetector continuousNoiseDetector;
EnvironmentalSoundDetector environmentalSoundDetector;
AudioEventRecognizer recognizer;
MotorDriver leftMotor(static_cast<uint8_t>(config::kMotorLeftPin), true);
MotorDriver rightMotor(static_cast<uint8_t>(config::kMotorRightPin),
                       config::kEnableRightMotor);
HapticEncoder hapticEncoder(leftMotor, rightMotor, runtimeSettings);
BleService bleService;
BatteryMonitor batteryMonitor;
WebDashboard webDashboard(runtimeSettings);
bool audioReady = false;
uint32_t lastMetricsLogMs = 0;

void logMetrics(const AudioFeatures& features, const AudioEvent& event,
                const uint32_t nowMs) {
  if (!runtimeSettings.serialMetricsEnabled ||
      nowMs - lastMetricsLogMs < config::kMetricsLogPeriodMs) {
    return;
  }
  lastMetricsLogMs = nowMs;
  const float signalToNoise =
      features.rms / std::max(features.noiseFloor, 0.000001F);
  Serial.printf(
      "rms=%.5f peak=%.5f gain=%.2f floor=%.5f snr=%.2f "
      "event=%s confidence=%.2f strength=%.2f "
      "bands=[%.3f,%.3f,%.3f,%.3f,%.3f,%.3f]\n",
      features.rms, features.peak, features.gain, features.noiseFloor,
      signalToNoise, audioEventName(event.type), event.confidence,
      event.acousticStrength,
      features.bands[0], features.bands[1], features.bands[2],
      features.bands[3], features.bands[4], features.bands[5]);
}

}  // namespace

void setup() {
  Logger::begin(config::kSerialBaud);
  delay(1000);
  Logger::info("VibHearing starting");

  if (!leftMotor.begin() || !rightMotor.begin()) {
    Logger::info("Motor PWM initialization failed");
  }
  recognizer.addDetector(humanVoiceDetector);
  // Stable machinery is a non-haptic veto class. It can outscore a
  // speech-like spectral frame without ever driving the motor.
  recognizer.addDetector(continuousNoiseDetector);
  if (config::kEnableExperimentalSoundClassifiers) {
    recognizer.addDetector(animalVocalizationDetector);
    recognizer.addDetector(impulsiveSoundDetector);
    recognizer.addDetector(tonalAlertDetector);
  }
  recognizer.addDetector(environmentalSoundDetector);
  bleService.begin();
  batteryMonitor.begin();
  webDashboard.begin();
  audioReady = audioEngine.begin();
}

void loop() {
  const uint32_t nowMs = millis();
  hapticEncoder.update(nowMs);
  bleService.update();
  webDashboard.update();
  if (!runtimeSettings.motorEnabled && hapticEncoder.isActive()) {
    hapticEncoder.stop();
  }

  if (!audioReady) {
    delay(250);
    return;
  }

  AudioFeatures features;
  if (!audioEngine.capture(features)) {
    return;
  }
  if (hapticEncoder.isActive()) {
    const AudioEvent noEvent{};
    webDashboard.updateAudio(features, noEvent);
    logMetrics(features, noEvent, nowMs);
    return;
  }
  const AudioEvent event = recognizer.recognize(features, nowMs);
  hapticEncoder.handleEvent(event);
  webDashboard.updateAudio(features, event);
  logMetrics(features, event, nowMs);
}
