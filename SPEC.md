# VibHearing Technical Specification

## Goals

- Convert meaningful environmental audio into repeatable vibration patterns.
- Let a cat learn associations gradually without continuously vibrating.
- Keep event detectors, feature extraction, and haptic encoding replaceable.
- Fail safely: motors default off, optional hardware defaults disabled, and no
  battery voltage is sampled without a defined divider.

This product is a sensory-substitution research device, not a hearing aid or
a safety-certified warning device.

## Hardware

Current target: Seeed Studio XIAO ESP32-C3, one MSM261S4030H0 I2S microphone,
one coin vibration motor, and one IRLML6344 MOSFET. GPIO assignments are:

| Function | GPIO | Status |
|---|---:|---|
| Left motor PWM | 7 | Active |
| Right motor PWM | 21 | Reserved, disabled |
| I2S BCLK | 4 | Active |
| I2S WS | 5 | Active |
| I2S DATA | 6 | Active |

The motor must use a flyback protection arrangement appropriate to the motor
and MOSFET circuit. Firmware PWM is 1 kHz at 8-bit resolution.

## Software

The application owns the control loop. `AudioEngine` captures frames and
extracts features; `AudioEventRecognizer` runs registered implementations of
`AudioEventDetector`; `HapticEncoder` maps accepted events to non-blocking
motor patterns. Drivers and services do not depend on detector
implementations. Compile-time product defaults live in `ProjectConfig.h`.

`HumanVoiceDetector` is the initial heuristic detector. Future statistical or
TinyML detectors implement the same detector interface and register without
changing the recognizer.

## Power

Power is a 3.7 V LiPo charged through the XIAO board. Runtime battery sensing
is unavailable until a resistor divider, ADC pin, calibration, and low-voltage
policy are specified. The `BatteryMonitor` API currently reports unavailable.
Motor current, thermal behavior, runtime, charging behavior, and enclosure
safety require bench validation before animal trials.

## Audio

The microphone is read through the ESP-IDF standard I2S channel API as 32-bit
words at 16 kHz. Its 24-bit payload is
shifted, normalized, and DC-centered into floating-point samples. Frames
contain 256 samples (16 ms). Per-frame features are RMS and absolute peak. A smoothed
automatic gain estimate targets a useful analysis level; it does not alter
the raw level metrics. A slow-rise/fast-fall noise-floor estimator provides a
local signal-to-noise reference.

## FFT

A dependency-free radix-2 FFT processes a Hann-windowed 256-sample frame. It
reports RMS spectral magnitude in six bands: 60-250, 250-500, 500-1000,
1000-2000, 2000-4000, and 4000-8000 Hz. The 62.5 Hz bin spacing means band
edges are approximate. Detector thresholds must be calibrated using captured
data from the final enclosure.

## Event recognition

Each detector receives `AudioFeatures` and returns confidence from 0 to 1.
The recognizer selects the highest confidence. The initial voice-activity
heuristic combines energy from 250 Hz to 4 kHz, a weighted contribution from
4-8 kHz for higher-pitched voice harmonics, broadband energy ratio,
signal-to-noise ratio, attack frames, and release frames. It recognizes voice
activity, not words, speakers, or semantic meaning.

Planned independent detectors include cat meow, dog bark, doorbell, alarm,
clap, keyboard, vacuum cleaner, car horn, and glass break. These labels are
part of the shared event vocabulary but are not yet claimed as implemented.

### Acoustic Haptic Language Model v0.2

The current model is a deterministic acoustic classifier, not a natural
language model and not a trained neural network. It provides a usable data and
behavior contract while labeled recordings are collected. It classifies every
salient sound through this ordered semantic vocabulary:

| Token | Evidence | Haptic rhythm | Meaning certainty |
|---|---|---|---|
| `human_voice` | Speech-band balance, temporal attack, adaptive SNR; includes a close/clipped path | two short pulses | Heuristic candidate |
| `animal_vocalization` | Mid/upper vocal energy, centroid, temporal persistence | three short pulses | Broad animal-vocalization candidate, not species identification |
| `impulsive_sound` | High peak-to-RMS crest factor | one crisp pulse | Broad transient class |
| `tonal_alert` | Sustained concentration in one non-bass band | long-short | Broad alarm/bell candidate |
| `continuous_noise` | Sustained level with limited frame-to-frame change | two long pulses | Broad machinery/noise candidate |
| `environmental_sound` | Any remaining signal above the adaptive noise floor | one short pulse | Presence known, source unknown |

All detectors implement `AudioEventDetector` and compete by confidence. The
specific classifiers intentionally score above the fallback when their
evidence is strong. `environmental_sound` is the coverage guarantee: a
salient sound is converted to haptics even when the firmware cannot label its
source honestly. Silence and signals below the adaptive threshold do not
vibrate. This prevents “detect all sounds” from becoming continuous motor
noise.

Audio capture continues during motor playback so adaptive audio state remains
current, but classification is suppressed while a haptic pattern is active.
This prevents the collar's own mechanical motor noise from recursively
creating new environmental events.

Current runtime policy enables `human_voice`, the non-haptic
`environmental_sound` observation fallback, and `continuous_noise` as a
non-haptic veto. The veto lets stable printer, fan, and machinery energy
outscore a speech-like frame. Experimental animal, impulsive, and tonal
classifiers remain compiled but unregistered because
their uncalibrated confidence could outscore real speech. The motor accepts
only `human_voice`; all other labels are telemetry-only until labeled data
supports safe activation.

Human voice attack requires five consecutive qualifying frames. The detector
also tracks smoothed RMS and spectral-band variation; stable machinery cannot
qualify solely because its frequency balance overlaps speech. Once speech is
active, a separate ten-frame release preserves natural syllable gaps without
allowing pre-activation candidates to accumulate over time.
The default voice SNR threshold is 2.0, selected after the target environment
showed printer-induced false candidates near the former 1.6 boundary. It
remains runtime-adjustable for controlled distance calibration.

The near-source path relaxes spectral balance only when signal dominance is
very high. It addresses close speech whose microphone frame is clipped or
spectrally broadened. It does not prove that the source is human. The label
must therefore be treated as provisional until the trained model replaces the
heuristic detector.

Classification confidence selects the token, while `acousticStrength`
represents received signal dominance from 4 to 20 dB above the learned noise
floor. The latter continuously controls bounded motor intensity. Neither
value is a distance measurement: a distant shout can be stronger than a near
whisper.

### Trained model contract

The production classifier should be a quantized audio model operating on a
log-mel spectrogram, with approximately 0.5-1.0 seconds of temporal context.
Its output tensor must use a versioned label map containing human voice,
animal vocalization, impulsive sound, tonal alert, continuous noise, other
salient sound, and background. Optional species or event labels may be added
only when evaluation data supports them. The adapter must expose the existing
`AudioEventDetector` contract so the recognizer and haptic language do not
depend on TensorFlow Lite Micro, Edge Impulse, or another runtime.

Training data must include the actual microphone and enclosure, close and far
sources, clipping, whispers, shouting, higher- and lower-pitched speakers,
cats, dogs, household appliances, alarms, impacts, outdoor sounds, motor
self-noise, collar motion, and silence. Splits must be separated by home,
speaker, and animal to prevent recording leakage. Required evaluation
includes per-class precision/recall, confusion matrices, background false
alerts per hour, missed urgent events, latency, RAM/flash, and energy use.
Until these measurements exist, the rule-based labels remain candidates and
must not be represented as reliable source identification.

## Haptics

Patterns are timed steps containing left and right intensities. Playback is
non-blocking. Human voice currently maps to two pulses on the left motor with
a retrigger guard. Pulse intensity varies continuously from 80 to 180 PWM
according to received acoustic strength above the adaptive noise floor;
rhythm and pulse width remain stable. This is relative acoustic salience, not
a physical distance estimate. Behavioral intent, vocabulary, and research
constraints are maintained in `docs/HAPTIC_LANGUAGE.md`.

## BLE

The BLE service boundary exists but radio initialization is disabled. Before
activation, define authenticated configuration, versioned characteristics,
safe intensity limits, persistence, update rate, privacy behavior, and a
recovery path for invalid settings.

## Wi-Fi and web dashboard

The ESP32-C3 joins the configured WPA network as a station and advertises the
`vibhearing.local` mDNS hostname. Credentials live only in the locally ignored
`include/WifiSecrets.h`; `WifiSecrets.example.h` documents the required
symbols without containing credentials. The firmware retries connection
without blocking audio processing.

The embedded HTTP dashboard has no external CDN dependency. `GET /api/status`
returns IP/RSSI, uptime, RMS, peak, adaptive gain, noise floor, SNR, event,
confidence, acoustic strength, six FFT bands, and all 256 normalized samples
from the latest DC-centered I2S frame. The browser polls this endpoint and
draws the waveform and bands locally. `GET /api/settings` reads and
`POST /api/settings` validates runtime settings.

Runtime controls are motor enable, serial telemetry enable, minimum voice
SNR, minimum voice-band ratio, close-source SNR, actuation confidence,
minimum/maximum PWM, and retrigger guard. PWM remains capped at the firmware
safe maximum. Disabling the motor stops an active pattern immediately.
Settings are volatile and reset to compiled safe defaults after reboot;
persistence requires a versioned schema, CRC, and atomic storage design.

The prototype HTTP interface is unauthenticated and unencrypted. It is
acceptable only on a trusted isolated development LAN and must not be exposed
through router port forwarding. Authentication, CSRF protection, TLS or an
authenticated application protocol, rate limiting, and credential
provisioning are required before field use.

## Configuration

Current hardware and conservative feature flags are compile-time constants.
Future runtime configuration should validate ranges, include a schema version
and CRC, use atomic persistence, and restore safe defaults on corruption.

## Future hardware

Two microphones, two motors, an IMU, battery monitor, BLE control, and a
production power subsystem are planned. Pin conflicts and ESP32-C3 resource
limits must be reviewed before enabling any addition.

## Future algorithms

Candidates include temporal smoothing, adaptive per-animal thresholds,
direction estimation with two microphones, motion-aware suppression, and
quantized TinyML models using TensorFlow Lite Micro or Edge Impulse. Model
work requires a labeled dataset representing the final microphone,
mechanical enclosure, homes, animals, and nuisance sounds.
