# Phase 1 Code Review (Bluepad32 + Xbox)

Date: 2026-04-20

## Summary

The Bluepad32 migration is structurally sound and the input layer compiles conceptually with the existing `RawInput` + `Intent` flow. I found and fixed three blockers before field testing:

1. `Intent.stop` could remain latched `true` after a disconnect.
2. Axis and trigger normalization could exceed expected ranges.
3. Loop timing could burst updates after long pauses.

## What was fixed

- `Intent` command mapping now explicitly sets:
  - `boost = RB`
  - `stop = B`
- Bluepad axis/trigger normalization now clamps values to safe ranges.
- Main fixed-step loop now prevents catch-up bursts after long stalls.

## Tonight's Bluetooth Validation Checklist

Run this in serial monitor while testing Xbox controller:

1. **Connection stability**
   - Power cycle ESP32 and reconnect controller 5+ times.
   - Confirm `Controller state` transitions are clean.

2. **Input sanity**
   - Verify sticks center near `0.00` (with deadzone applied).
   - Verify full stick throws approach `-1.00` to `1.00`.
   - Verify triggers approach `0.00` to `1.00`.

3. **Safety behavior**
   - Disconnect controller while moving stick.
   - Confirm intent returns to zero quickly.

4. **Stop + boost mapping**
   - Press `B` and verify stop command behavior.
   - Press `RB` and verify boost flag toggles.

5. **Jitter check**
   - Hold stick at a constant value for 10s.
   - Confirm value is stable and not noisy.

## Optional next hardening (Phase 1.5)

- Add explicit `CONNECTING` state timeout handling.
- Add periodic raw input dump behind a compile-time debug flag.
- Add connection watchdog (no input for `>500ms` => stop intent).
