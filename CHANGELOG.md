# Changelog

All notable changes to this project will be documented in this file. The
format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and the project intends to use Semantic Versioning once releases begin.

## [Unreleased]

### Added

- Layered audio, FFT, event recognition, haptic, motor, BLE, battery,
  configuration, and logging modules.
- I2S capture with RMS, peak, adaptive gain, and noise-floor features.
- ESP-IDF standard I2S channel API without the deprecated legacy driver.
- Six-band radix-2 FFT analysis and an independent detector interface.
- Initial heuristic human voice activity detector with temporal hysteresis.
- Non-blocking two-pulse voice haptic pattern and future right-motor support.
- Continuous safe-range haptic intensity derived from received acoustic
  strength above the adaptive noise floor.
- Acoustic Haptic Language v0.2 with independent broad sound classifiers,
  distinct haptic tokens, and an all-salient-sound fallback.
- Local Wi-Fi dashboard, mDNS discovery, JSON telemetry/settings APIs, live
  256-sample waveform plot, FFT visualization, and device status.
- Product specification, roadmap, haptic language, project overview, and
  living development journal.

### Changed

- Replaced the single-file microphone experiment with a maintainable
  application pipeline.
- Centralized hardware pins and operational defaults in `ProjectConfig.h`.
- Increased voice detector range with a lower adaptive signal-to-noise
  threshold, longer temporal confirmation, and weighted 4-8 kHz harmonics.
- Removed per-frame microphone DC offset before level and FFT analysis.
- Added a high-dominance path for nearby or clipped vocal audio and raised
  specific voice confidence above the generic sound fallback.
- Suppressed event classification during motor playback to prevent acoustic
  self-trigger loops while keeping audio capture active.
- Restricted motor activation to `human_voice` and disabled registration of
  uncalibrated competing classifiers while retaining environmental telemetry.
- Moved voice and haptic thresholds into validated runtime settings and made
  web motor disable stop active playback immediately.

### Fixed

- Removed corrupted non-ASCII comments and the ambiguous board-alias motor
  pin from the original prototype.
- Fixed an extra JavaScript closing brace that prevented all dashboard live
  values and plots from rendering.
- Prevented intermittent printer noise from accumulating non-consecutive voice
  attack frames; added temporal spectral variation and a continuous-noise veto.
- Raised the evidence-based default voice SNR threshold from 1.6 to 2.0 after
  live printer tests showed boundary-triggered false speech events.
