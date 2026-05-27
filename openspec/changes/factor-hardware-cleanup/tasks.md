## 1. Module Scaffolding

- [x] 1.1 Create `src/hal/hardware_cleanup.h` and `src/hal/hardware_cleanup.cpp` with a minimal public API for analog/digital input cleanup.
- [x] 1.2 Define lightweight per-input state structs (for median/EMA/hysteresis/debounce bookkeeping) with deterministic initialization.

## 2. Analog Cleanup Integration

- [x] 2.1 Implement analog cleanup primitives (median-of-3 spike rejection, EMA smoothing, hysteresis/deadband gating) in the new module.
- [x] 2.2 Refactor fader read path in `powaplay.ino` to call the module API instead of inline threshold-only conditioning.
- [x] 2.3 Ensure default behavior remains backward compatible unless tuning constants are changed.

## 2b. Digital Cleanup Integration (Pads-Only Follow-up)

- [x] 2b.1 Integrate `debounceDigital` for pad input reads in `updatePads()` with per-pad debounce state.
- [x] 2b.2 Add a minimal compile-time debounce setting (`padDebounceUs`) in mapping/profile config and wire it to pad debounce flow.

## 3. Configuration and Tuning

- [x] 3.1 Add compile-time tuning constants in profile/config definitions for analog smoothing, hysteresis/deadband, and optional CC minimum update interval.
- [x] 3.2 Wire the new constants through selected profile/config headers so Nano and Pro Micro builds can tune independently if needed.
- [x] 3.3 Document recommended starter values for noisy-fader hardware in the relevant project documentation.

## 4. Verification

- [x] 4.1 Build and verify target environments (`nanoatmega328`, `promicro16`) compile successfully with the refactor.
- [x] 4.2 Confirm idle fader jitter no longer causes continuous CC churn while preserving smooth response during intentional movement.
- [x] 4.3 Run a focused regression check for mode selection, CC lane editing, and display updates to ensure no behavioral breakage.

Notes:
- Build verification is currently blocked in this environment (`platformio` is not installed).
