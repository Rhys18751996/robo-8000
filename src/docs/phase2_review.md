# Phase 2 Code Review (Mapping System + GPIO Actions)

Date: 2026-04-20

## Summary

Phase 2 mapping is now separated cleanly from controller input transport and control loop timing. `RawInput` is translated into `Intent` using a JSON-driven mapping config loaded once at startup, then consumed by `control.cpp` for behavior and GPIO outputs.

This phase now includes three mapped output action patterns:

1. **Pulse action**: one button press creates a timed HIGH pulse on a mapped pin.
2. **Toggle action**: one button press flips pin state HIGH/LOW.
3. **Hold action**: pin remains HIGH only while button is held.

## What was implemented

- Added `MappingConfig` and mapping translation flow:
  - `initMapping()`
  - `mapInputToIntent()`
  - `getMappingConfig()`
- Added default JSON mapping config for:
  - Axis mappings (`linear`, `angular`)
  - Basic buttons (`boost`, `stop`)
  - Action buttons (`pulse`, `toggle`, `hold`)
  - Output pin config (`pulsePin`, `togglePin`, `holdPin`, `pulseDurationMs`)
  - Filters (`deadzone`, `smoothingAlpha`)
  - Inversion (`invert.linear`, `invert.angular`)
- Extended `Intent` with:
  - `pulseAction`
  - `toggleAction`
  - `holdAction`
- Updated `outputControl()` to handle edge-detected pulse/toggle + direct hold mirroring.

## Phase 2 Manual Validation Checklist

Run with Serial Monitor open (`115200`) and controller connected.

1. **Mapping loads at boot**
   - Reboot ESP32.
   - Confirm mapping-loaded log appears once.
   - Confirm output pin init log appears with expected pins.

2. **Axis mapping sanity**
   - Move left stick Y and confirm `Intent.linear` changes.
   - Move left stick X and confirm `Intent.angular` changes.
   - Verify center returns near `0.00` with deadzone.

3. **Stop + boost mapping sanity**
   - Press `B` (default stop) and confirm movement intent is zeroed.
   - Press `RB` (default boost) and confirm boost behavior flag path is active.

4. **Pulse action validation**
   - Press `Y` (default pulse button) once.
   - Verify `pulsePin` goes HIGH briefly, then returns LOW automatically.
   - Press repeatedly and verify each press creates a fresh pulse.

5. **Toggle action validation**
   - Press `X` (default toggle button) once → pin should change state.
   - Press again → pin should return to previous state.
   - Hold button down: state should not retrigger continuously (edge-detected).

6. **Hold action validation**
   - Hold `A` (default hold button): `holdPin` should stay HIGH.
   - Release `A`: `holdPin` should go LOW immediately.

7. **Disconnect safety**
   - Move stick, then power off controller or disconnect Bluetooth.
   - Confirm movement intent resets to zero and action outputs stop safely.

8. **Noise / stability check**
   - Keep controller connected for 10+ minutes idle.
   - Confirm no random GPIO toggles/pulses from noise.

## Suggested quick hardware test method

- Put an LED + resistor on each configured output pin:
  - Pulse pin LED should blink briefly per press.
  - Toggle pin LED should latch on/off each press.
  - Hold pin LED should track button hold exactly.

## Optional next hardening (Phase 2.5)

- Add pin conflict validation (prevent same pin used by multiple actions unless explicitly allowed).
- Add mapping schema validation and clear error logs for invalid button/axis names.
- Add compile-time debug mode to print action state transitions.
- Move default mapping JSON to NVS-backed config in Phase 3.
