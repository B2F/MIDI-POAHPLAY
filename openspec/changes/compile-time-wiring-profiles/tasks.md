## 0. Phase 1 Scope Lock (Serial Nano Parity)

- [x] 0.1 Keep phase 1 limited to Nano (`nanoatmega328`) + serial MIDI transport at `38400` baud; do not introduce Pro Micro or USB transport behavior changes in this phase.
- [x] 0.2 Define and preserve parity defaults: MIDI channel, base note/transpose reference, CC defaults/presets, and mode switch behavior must match current firmware behavior.
- [x] 0.3 Keep `powaplay.ino` as thin orchestration only after extraction; core behavior must not depend on direct pin constants.

## 1. Architecture Extraction and Interfaces

- [x] 1.1 Create `src/core`, `src/hal`, `src/platform`, `src/profiles`, and `src/config` module structure with a thin `.ino` orchestration entrypoint.
- [x] 1.2 Extract MIDI transport calls behind `midi_iface` and replace direct transport usage in core behavior paths.
- [x] 1.3 Extract display and IO access behind `display_iface` and `io_iface` to remove direct pin usage from core logic.

## 2. Compile-Time Profile System

- [x] 2.1 Add typed board and wiring profile schemas with capability fields and required signal definitions.
- [x] 2.2 Implement default tracked profiles for Nano and Pro Micro clone targets.
- [x] 2.3 Add compile-time profile validation checks that fail fast for unsupported capabilities.

## 3. Transport and Build Selection

- [x] 3.1 Implement platform transport adapters for serial MIDI and USB MIDI backends behind the shared MIDI interface.
- [x] 3.2 Add centralized `build_config.h` selectors for board, wiring, and mapping profiles.
- [x] 3.3 Add build-target definitions (Arduino CLI and/or PlatformIO environments) so switching targets requires no source edits.

## 4. Pluggable Mapping and Local Overrides

- [x] 4.1 Add default MIDI mapping profile definitions separated from core behavior logic.
- [x] 4.2 Add optional local override entrypoints (`build_config.local.h`, local mapping headers) and exclude them from version control.
- [x] 4.3 Implement default fallback behavior when local override files are absent.

## 5. Validation and Documentation

- [x] 5.1 Add dual-target compile checks for Nano and Pro Micro profiles.
- [ ] 5.2 Run hardware probe verification for divergent pins and document expected profile constraints.
- [x] 5.3 Update README/developer docs with profile selection, mapping plugin workflow, and troubleshooting for alias mismatches.

## 6. Manual Verification Checklist (Use After Each Upload)

- [x] 6.1 Boot and transport sanity: device boots cleanly, serial monitor is stable at `38400`, DAW/bridge receives MIDI traffic.
- [x] 6.2 Standard mode parity: pads trigger expected notes, velocity and octave controls respond, scale/chord selectors wrap as before.
- [x] 6.3 CC mode parity: lane values update from encoders/faders, lane selection via encoder push matches prior behavior, preset CC labels/selection remain correct.
- [x] 6.4 Arp mode parity: arp type/rate edits work, repeat lock workflows behave consistently, internal timing remains stable.
- [ ] 6.5 Clock sync parity: incoming MIDI clock start/stop/continue drives arp timing as before when transport sync is enabled.
- [x] 6.6 Ultrasonic parity: target CC selection works, distance-to-CC smoothing/deadband behavior is stable, update cadence remains usable.
- [x] 6.7 Init/reset parity: setup mode parameter edits work, reinit workflow remains functional.
- [ ] 6.8 Regression log: for each upload, record profile selected, build command, observed differences, and pass/fail status.
