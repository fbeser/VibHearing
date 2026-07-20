# Haptic Language

## Project philosophy

VibHearing should communicate a few consistent, learnable cues rather than
translate every sound. Patterns are associations, not replicas of audio. The
design prioritizes low surprise, low exposure, distinct rhythms, and stable
meaning. Any animal study requires veterinary and animal-behavior guidance,
gradual introduction, positive reinforcement, and an immediate stop condition
for stress or avoidance.

## Learning stages

1. **Habituation:** wear the powered-off, correctly fitted collar for short,
   supervised periods.
2. **Single cue:** introduce the gentlest detectable pulse paired with an
   immediate positive, visible event.
3. **Stable association:** repeat one cue consistently across days while
   recording behavior and stopping on discomfort.
4. **Second cue:** add only after the first cue produces a calm, repeatable
   response; use clearly different rhythm, not simply more force.
5. **Generalization:** test the same event at new locations and sound levels
   without changing its pattern.
6. **Adaptive use:** adjust conservative intensity and repetition limits per
   animal under expert review; never silently change vocabulary meaning.

Progress is individual and reversible. No response is a valid outcome.

## Vocabulary

| Event | Proposed pattern | Status |
|---|---|---|
| Human voice activity | medium-short, pause, medium-short | Implemented prototype |
| Animal vocalization candidate | three gentle short pulses | Implemented heuristic |
| General environmental sound | one short pulse | Implemented fallback |
| Tonal alert / bell candidate | one long then one short pulse | Implemented heuristic |
| Impulse / clap candidate | one crisp pulse | Implemented heuristic |
| Continuous machinery/noise | two longer pulses | Implemented heuristic |
| Car horn | long pulse plus directional cue | Future research idea |

Heuristic labels are candidates, not proven source identification.
Unimplemented patterns are hypotheses, not promises. Similar rhythms must be
confusion-tested before expanding the vocabulary.

## Pattern timing

The prototype voice pattern is 110 ms on, 70 ms off, and 110 ms on. A 450 ms
retrigger guard prevents continuous replay. Future timing should normally use
perceptibly distinct groups while limiting total duty cycle. Long and
repeating warnings require explicit maximum-duration and cooldown policies.

## Intensity

Intensity is an 8-bit PWM command, not a calibrated physical quantity. Human
voice intensity is dynamically mapped from received signal-to-noise strength
to 80-180/255. A distant shout can therefore produce a stronger cue than a
nearby whisper; this is acoustic salience, not measured distance. Rhythm and
pulse width remain unchanged so the learned event identity stays stable. The
limits remain subject to reduction during bench and behavioral calibration.
Motor, mounting, fur, fit, battery voltage, and anatomy
all change perceived intensity. Establish minimum detectable and maximum
comfortable levels per device and animal; firmware must enforce conservative
caps.

## Pulse width

Pulse width carries category information and should remain independent of
sound loudness where possible. Very short commands may not overcome motor
startup inertia; long commands increase habituation, heat, and discomfort.
Measure the actual mechanical envelope in the production mounting.

## Rhythm

Rhythm is the primary vocabulary dimension because it can distinguish events
without escalating force. Prefer small families such as single, double,
triplet, and long-short. Reserve urgent rhythms sparingly and avoid patterns
that resemble the device's startup or error feedback.

## Future left/right semantics

Potential meanings include sound direction, event category, or urgency. Do not
mix these conventions. Direction requires two microphones, calibration, and
evidence that the spatial cue is stable on a moving collar. A simpler category
mapping may be more learnable. The right motor remains disabled in current
firmware.

## Adaptive learning

Possible adaptation includes per-animal intensity bounds, time-of-day
sensitivity, event confidence thresholds, cooldowns, and a caregiver-confirmed
training mode. Adapt slowly, log every change, preserve a safe reset, and never
let a classifier raise intensity beyond a validated limit. Favor adaptation of
detection thresholds and repetition over changes to learned rhythm semantics.

## Research ideas

- Compare fixed patterns with caregiver-triggered paired training.
- Measure recognition using behavior agreed in advance with a specialist.
- Study false alerts, habituation, startle, sleep disruption, skin condition,
  and avoidance as first-class outcomes.
- Test whether event category is more useful than left/right direction.
- Explore motor self-noise cancellation and post-vibration detector blanking.
- Compare amplitude, rhythm, and location while holding total energy constant.
- Determine the smallest useful vocabulary and the cost of adding each cue.
- Use privacy-preserving feature logs for threshold tuning.
- Compare confidence-dependent repetition with the implemented
  acoustic-strength-dependent intensity.
- Review tactile sensitivity and safe wear schedules with veterinarians.
