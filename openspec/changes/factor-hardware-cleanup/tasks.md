## 1. Module Scaffolding

- [ ] 1.1 Create `src/hal/hardware_cleanup.h` and `src/hal/hardware_cleanup.cpp` with a minimal public API for analog/digital input cleanup.
- [ ] 1.2 Define lightweight per-input state structs (for median/EMA/hysteresis/debounce bookkeeping) with deterministic initialization.

## 2. Analog Cleanup Integration

- [ ] 2.1 Implement analog cleanup primitives (median-of-3 spike rejection, EMA smoothing, hysteresis/deadband gating) in the new module.
- [ ] 2.2 Refactor fader read path in `powaplay.ino` to call the module API instead of inline threshold-only conditioning.
- [ ] 2.3 Ensure default behavior remains backward compatible unless tuning constants are changed.

## 3. Configuration and Tuning

- [ ] 3.1 Add compile-time tuning constants in profile/config definitions for analog smoothing, hysteresis/deadband, and optional CC minimum update interval.
- [ ] 3.2 Wire the new constants through selected profile/config headers so Nano and Pro Micro builds can tune independently if needed.
- [ ] 3.3 Document recommended starter values for noisy-fader hardware in the relevant project documentation.

## 4. Verification

- [ ] 4.1 Build and verify target environments (`nanoatmega328`, `promicro16`) compile successfully with the refactor.
- [ ] 4.2 Confirm idle fader jitter no longer causes continuous CC churn while preserving smooth response during intentional movement.
- [ ] 4.3 Run a focused regression check for mode selection, CC lane editing, and display updates to ensure no behavioral breakage.
