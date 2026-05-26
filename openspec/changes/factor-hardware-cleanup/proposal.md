## Why

Hardware-facing setup and cleanup behavior is currently spread across the main firmware file, which makes it harder to tune noisy inputs and maintain board-specific wiring behavior safely. We need a focused hardware cleanup module now to improve readability and make filtering/debounce work easier to evolve without destabilizing core mode logic.

## What Changes

- Introduce a dedicated hardware cleanup module/file that centralizes input conditioning responsibilities (fader smoothing, debounce, and CC-friendly stabilization helpers).
- Refactor `powaplay.ino` to call this module instead of keeping cleanup logic inline.
- Keep existing functional behavior compatible by default, while allowing profile-driven tuning for noisy hardware.
- Add clear integration boundaries so board/wiring profiles remain the source of pin mapping truth.

## Capabilities

### New Capabilities
- `hardware-input-cleanup-module`: Provide a reusable module for hardware input cleanup (fader filtering, debounce, and stabilization primitives) consumed by the main app loop.

### Modified Capabilities
- None.

## Impact

- Affected code: `powaplay.ino`, new files under `src/` for hardware cleanup logic, and mapping/profile configuration touchpoints for tuning constants.
- No protocol/API breaking changes expected; MIDI behavior should remain functionally equivalent with improved stability on noisy controls.
- Improves maintainability for Nano and Pro Micro profile variants by separating cleanup concerns from mode/state logic.
