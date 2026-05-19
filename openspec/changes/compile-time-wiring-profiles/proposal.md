## Why

The current firmware is monolithic and couples musical behavior with board pins and transport details, making Nano and Pro Micro support fragile. We need a compile-time profile architecture so one shared core can support multiple board/wiring variants without branch drift or macro sprawl.

## What Changes

- Introduce a shared core/hal/platform/profile project structure that separates musical logic from hardware bindings.
- Add compile-time board, wiring, and MIDI transport selection through a single build configuration boundary.
- Add wiring profile plugins so users can provide local pin maps without versioning custom maps in the repository.
- Define compatibility rules for Nano and Pro Micro profiles, including alias handling for divergent pins and serial-vs-USB transport.
- Add validation workflow for profile probing and dual-target compile checks.

## Capabilities

### New Capabilities
- `compile-time-board-profiles`: Board and wiring profile selection at build time with typed profile contracts and central configuration.
- `pluggable-midi-mapping`: External/local MIDI mapping and wiring plugin selection without modifying shared core files.

### Modified Capabilities
- None.

## Impact

- Affected code: `powaplay.ino` and new modular directories under `src/` for core, hal, platform, profiles, and config.
- Build/tooling impact: adds compile-time flags/environment targets (Arduino CLI and/or PlatformIO matrix).
- Developer workflow impact: board/wiring changes move from in-code `#if` edits to profile-driven selection.
