# Roadmap

Checkboxes describe verified outcomes, not merely code presence.

## Phase 1 - Hardware

- [x] Record current pin assignment and component set.
- [x] Drive one motor through PWM with a default-off state.
- [ ] Validate motor current, MOSFET temperature, flyback protection, and EMI.
- [ ] Measure LiPo runtime and charging behavior in the enclosure.

## Phase 2 - Audio

- [x] Capture normalized I2S frames.
- [x] Calculate RMS, peak, adaptive gain, and noise floor.
- [ ] Record representative calibrated datasets from the final enclosure.

## Phase 3 - FFT

- [x] Implement Hann-windowed radix-2 FFT and six frequency bands.
- [ ] Validate band magnitude and timing against reference signals.
- [ ] Profile CPU, stack, heap, and sustained frame loss.

## Phase 4 - Event recognition

- [x] Define an independent detector interface and registry.
- [x] Add an initial heuristic human voice activity detector.
- [x] Add broad animal-vocalization, impulse, tonal, continuous-noise, and
  environmental fallback tokens.
- [ ] Calibrate voice detection and measure false positive/negative rates.
- [ ] Collect labeled human, animal, household, close, distant, and clipped
  recordings from the target enclosure.
- [ ] Add and validate further detectors one at a time.

## Phase 5 - Haptic language

- [x] Define a non-blocking pattern representation and initial voice pattern.
- [x] Document vocabulary principles and staged learning.
- [ ] Run veterinarian-approved bench and behavioral studies.
- [ ] Establish per-animal intensity and exposure limits.

## Phase 6 - BLE

- [x] Create a disabled service boundary.
- [ ] Specify a versioned, authenticated configuration protocol.
- [ ] Implement provisioning, telemetry, and safe settings persistence.
- [x] Add a development Wi-Fi dashboard with live waveform, FFT, telemetry,
  and validated volatile controls.
- [ ] Add authentication and safe network provisioning before field use.

## Phase 7 - Phone app

- [ ] Define onboarding, calibration, telemetry, and accessibility flows.
- [ ] Build a cross-platform prototype after the BLE protocol stabilizes.

## Phase 8 - TinyML

- [ ] Establish data governance, labels, metrics, and train/test splits.
- [ ] Compare classical detectors with quantized candidate models.
- [ ] Integrate the selected model behind `AudioEventDetector`.

## Phase 9 - Production

- [ ] Design the production PCB, enclosure, strain relief, and service plan.
- [ ] Complete animal welfare, reliability, EMC, radio, battery, and regulatory
  reviews for intended markets.
- [ ] Add manufacturing tests, signed releases, traceability, and field-update
  recovery.
