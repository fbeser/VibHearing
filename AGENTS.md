# Development Journal

This file is cumulative. Add new facts and dated session entries; do not erase
history. Keep claims separate from work that still needs hardware validation.

## Completed

- Created the first modular firmware architecture.
- Implemented I2S frames, level features, adaptive levels, six FFT bands, a
  detector registry, heuristic voice activity, and non-blocking haptics.
- Added broad acoustic tokens and an environmental fallback so salient,
  unclassified sounds still produce a distinct haptic cue.
- Added safe disabled boundaries for BLE, right motor, and battery sensing.
- Established product and haptic documentation.

## Current

- Phase 1-5 firmware foundations exist.
- Only the left motor and one microphone are enabled.
- Human voice detection is a sensitivity-tuned but still uncalibrated
  heuristic.

## Next

- Bench-build, upload, and capture serial data with silence, speech, motor
  vibration, household sounds, and different distances.
- Validate FFT output with generated tones and check real-time frame loss.
- Tune thresholds from labeled recordings instead of anecdotes.
- Measure motor electrical and mechanical safety before animal exposure.

## Problems

- Hardware-in-loop evidence is limited to short serial smoke tests; controlled
  stimulus and electrical measurements have not been recorded.
- Motor vibration may mechanically couple into the microphone and retrigger
  detection; enclosure measurements are required.
- A 256-point FFT provides coarse 62.5 Hz resolution.
- Battery sensing has no defined circuit or pin.
- Increased long-range sensitivity may also increase household false positives;
  labeled distance tests are required.
- Dashboard JSON currently uses Arduino `String`; long-duration polling needs
  heap-fragmentation and audio-frame-loss measurement.
- Wi-Fi radio activity may alter power use and microphone noise; this has not
  been characterized on battery power.

## Ideas

- Freeze or discount audio classification while a motor pulse is active.
- Store compact feature telemetry rather than private raw household audio.
- Use the IMU to distinguish collar motion from acoustic events.
- Add detector arbitration so urgent alarms can preempt informational events.
- Explore direction semantics only after two-microphone measurements prove
  useful.

## Testing

- Required for meaningful changes: build, upload, observe the serial monitor,
  debug, and repeat until the acceptance criteria pass.
- Document the board, firmware revision, recording scenario, observed metrics,
  and failures. A successful compile is not a hardware test.
- Never bench-test an energized motor while the collar is worn by an animal.
- 2026-07-17: clean PlatformIO release build passed for XIAO ESP32-C3 using
  Arduino-ESP32 3.2.0 / ESP-IDF libraries 5.4. Firmware uploaded to the
  connected board on COM11. An eight-second serial observation showed
  continuous RMS, peak, gain, noise-floor, and six-band output with no I2S
  errors; the quiet-room event remained `none`. RAM was 16,088 bytes (4.9%)
  and flash was 316,276 bytes (16.1%). Controlled voice stimuli, motor
  observation, long-duration operation, and electrical measurements remain
  unverified.

## Architecture Notes

- Detectors implement `AudioEventDetector`; the recognizer does not know their
  concrete type. TinyML must enter through this interface.
- `AudioFeatures` is the stable boundary between signal processing and event
  recognition.
- Haptic playback is a non-blocking state machine, allowing audio capture to
  continue while patterns run.
- Event rhythm identifies meaning while received acoustic strength controls
  bounded intensity; strength is not presented as a distance measurement.
- Feature flags default optional or electrically undefined hardware to off.
- Serial commands are isolated in `SerialCommandProcessor`; the application
  loop only polls it, so future commands do not spread parsing logic.

## Session History

### 2026-07-20 - Open-source project presentation

- Rewrote the repository README as a public-facing project overview rather
  than a developer-only build note.
- Added project motivation, platform vision, status and hardware tables,
  architecture and dashboard descriptions, safe setup instructions, research
  principles, documentation links, contribution guidance, and explicit
  experimental-use disclaimers.
- Kept current capabilities separate from research directions: the README
  states that one microphone and one motor are active, human voice recognition
  is an uncalibrated heuristic, TinyML is not integrated, and BLE and battery
  sensing remain disabled.

### 2026-07-17 - Product foundation

- Reviewed the original XIAO ESP32-C3 RMS/peak prototype.
- Added the modular capture-to-haptics pipeline and initial documentation.
- Migrated microphone capture to the ESP-IDF standard I2S channel API.
- Deferred BLE radio, battery ADC, additional event types, and behavioral
  claims until their protocols, circuitry, or evidence exist.
- Completed a clean build, board upload, and short serial smoke test; recorded
  the remaining hardware validation rather than treating the smoke test as
  calibration.

### 2026-07-17 - Voice sensitivity tuning

- User observation: lower-pitched close speech triggered, while higher-pitched
  speech and more distant speech did not.
- Added frame DC removal, included weighted 4-8 kHz voice harmonics, reduced
  the adaptive signal-to-noise threshold from 2.5 to 1.6, and increased the
  attack from three to five frames to limit short noise triggers. An initial
  1.35 threshold produced excessive ambient activity during the serial smoke
  test and was not retained.
- The balanced 1.6 profile was uploaded successfully. During a six-second
  uncontrolled observation most logged frames were `none`, with two brief
  `human_voice` reports. Because the acoustic scene was not labeled, these
  reports cannot be classified as true or false positives.
- Added signal-to-noise ratio to serial metrics for controlled distance and
  pitch calibration.

### 2026-07-17 - Dynamic haptic intensity

- Replaced the fixed 145 PWM voice command with a continuous 80-180 mapping
  from 4-20 dB signal dominance above the adaptive noise floor.
- Kept the learned double-pulse timing constant and exposed normalized event
  strength in serial telemetry.
- PlatformIO build passed (16,088 bytes RAM, 318,376 bytes flash). Two upload
  attempts were blocked because COM11 was held open by another serial client;
  this firmware revision has not yet been verified on the board.

### 2026-07-17 - Acoustic Haptic Language v0.2

- Added independent animal-vocalization candidate, impulsive, tonal-alert,
  continuous-noise, and environmental fallback detectors.
- Added a close/high-dominance voice path for broadened or clipped frames.
- Assigned distinct rhythms while preserving dynamic acoustic-strength
  intensity and documented the trained TinyML replacement contract.
- Explicitly kept heuristic source labels provisional; real human/animal
  reliability requires labeled target-hardware recordings.
- Prevented motor self-noise feedback by retaining audio capture but pausing
  classification during active haptic patterns.
- Clean build and COM11 upload passed at 16,136 bytes RAM (4.9%) and 321,038
  bytes flash (16.3%). A seven-second uncontrolled serial test showed mostly
  `none` and two low-strength `environmental_sound` fallback events without
  I2S errors. Source-label accuracy remains unverified without labeled input.
- The motor-feedback suppression revision also built and uploaded successfully
  (321,064 bytes flash). A four-second serial smoke test showed live `none`,
  `environmental_sound`, and `human_voice` telemetry without I2S errors; the
  acoustic scene was uncontrolled, so this verifies execution, not labels.

### 2026-07-17 - Human-only actuation policy

- User reported that speech was still not labeled `human_voice`; identified
  that higher-confidence experimental classifiers could win arbitration.
- Disabled experimental classifier registration by default, retained the
  low-priority environmental telemetry fallback, and restricted motor
  actuation to `human_voice` only.
- Raw target-microphone recordings or labeled feature telemetry are still
  required before further evidence-based threshold or model tuning.
- Firmware built and uploaded successfully to COM11 (16,136 bytes RAM,
  321,024 bytes flash). A five-second uncontrolled observation produced live
  `none` and two temporally held `human_voice` reports with no I2S errors;
  because no stimulus labels were recorded, this is only a runtime check.

### 2026-07-17 - Wi-Fi development dashboard

- Added non-blocking station Wi-Fi, mDNS discovery, embedded HTTP UI, JSON
  telemetry/settings endpoints, full-frame normalized waveform plotting, FFT
  bands, and device status.
- Moved voice/haptic tuning into live validated runtime settings. Settings are
  intentionally volatile; motor PWM remains capped at 180 and web disable
  stops active playback.
- Kept the real Wi-Fi credentials in ignored `WifiSecrets.h` with a safe
  example header. The prototype HTTP surface is restricted by documentation
  to a trusted development LAN because authentication is not implemented.
- Initial dashboard build/upload passed; device joined Wi-Fi at
  `192.168.50.162` with mDNS name `vibhearing.local`. Dashboard, status GET,
  settings GET, and unchanged-values settings POST all returned HTTP 200.
- Final full-frame build/upload passed at 43,992 bytes RAM (13.4%) and
  1,062,568 bytes flash (54.0%). The live status API returned six FFT bands,
  exactly 256 waveform samples, RSSI -29 dBm, and current audio telemetry.
- User reported an empty dashboard. The device API was healthy, but a Node
  syntax check found an extra closing brace in the JavaScript polling
  function; fixed the client-side parser failure.
- Corrected firmware built and uploaded successfully. The served dashboard
  returned HTTP 200, its extracted JavaScript passed `node --check`, and the
  live API returned 256 waveform samples after reboot.

### 2026-07-17 - Persistent machinery rejection

- User reported that a 3D printer four to five meters away eventually became
  `human_voice`, stopped doing so briefly after real speech, then returned.
- Found that pre-activation voice frames accumulated across non-candidate gaps
  instead of requiring a consecutive attack.
- Reset incomplete attacks immediately, added smoothed RMS/spectral variation
  as a voice requirement, and permanently registered continuous machinery as
  a higher-confidence, non-haptic veto class.
- A 30-second API sample still contained 2 `human_voice` results out of 60;
  a second 30-second capture found release-tail SNR values of 1.20 and 1.38.
  Prior labeled speech was 3-11 SNR, so the default voice threshold was raised
  from 1.6 to 2.0 for the next validation pass.
- The 2.0-SNR firmware built and uploaded successfully (44,040 bytes RAM,
  1,063,116 bytes flash). In the same running-printer environment, the next
  30-second/60-sample API test produced 55 `none`, 5 non-haptic
  `environmental_sound`, and zero `human_voice`, with no request failures.

### 2026-07-23 - Serial command foundation

- Added a bounded, non-blocking serial command processor that accepts commands
  by line ending or a short receive-idle timeout.
- Added `r` and `reset` as restart commands; unknown and oversized input is
  rejected without changing device state.
- Release build passed at 44,072 bytes RAM (13.4%) and 1,063,630 bytes flash
  (54.1%), then uploaded successfully to COM11. Live serial tests confirmed
  rejection of `xyz`, restart acknowledgement for both `r` with a line ending
  and bare `r` via the idle timeout, followed by fresh startup and I2S-ready
  messages.

### 2026-07-23 - English dashboard copy

- Converted every user-visible dashboard heading, label, connection state,
  unit suffix, helper sentence, and settings result message from Turkish to
  English. API field names and firmware behavior were unchanged.
- Release build and COM11 upload passed at 44,072 bytes RAM (13.4%) and
  1,063,638 bytes flash (54.1%). The live device page contained the required
  English copy, no searched Turkish UI terms, valid JavaScript, and the status
  API continued to return all 256 waveform samples.

### 2026-07-23 - Prototype media

- Moved the root prototype GIF and MP4 into `assets/` with descriptive,
  stable English filenames.
- Added the collar-form-factor GIF near the README introduction and a direct
  MP4 link in a dedicated working-demo section.
- Kept the media descriptions within current evidence: early experimental
  sensory-substitution hardware, uncalibrated heuristic voice detection, no
  medical-device claim, no reliability or product-safety claim, and no
  implication of active-motor testing on an animal.
- Verified GIF and MP4 file signatures, exact README path casing, one reference
  per asset, one working-demo heading, and removal of the original root paths.
  The GIF is 7,111,706 bytes and the MP4 is 28,042,050 bytes; the latter is
  linked instead of embedded because inline video behavior varies by client.
- Before publication, the GIF was optimized to 3,313,109 bytes (3.16 MiB)
  without changing its path or GIF89a validity.

### 2026-07-24 - GitHub Sponsors integration

- Added `.github/FUNDING.yml` for the `fbeser` GitHub Sponsors profile.
- Added README sections explaining how sponsorship supports prototype
  hardware, testing, firmware, signal processing, and documentation.
- Kept the funding language explicitly experimental and avoided medical,
  veterinary, safety, or product-readiness claims.
