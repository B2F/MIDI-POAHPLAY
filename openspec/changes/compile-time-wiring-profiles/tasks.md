## 1. Architecture Extraction and Interfaces

- [ ] 1.1 Create `src/core`, `src/hal`, `src/platform`, `src/profiles`, and `src/config` module structure with a thin `.ino` orchestration entrypoint.
- [ ] 1.2 Extract MIDI transport calls behind `midi_iface` and replace direct transport usage in core behavior paths.
- [ ] 1.3 Extract display and IO access behind `display_iface` and `io_iface` to remove direct pin usage from core logic.

## 2. Compile-Time Profile System

- [ ] 2.1 Add typed board and wiring profile schemas with capability fields and required signal definitions.
- [ ] 2.2 Implement default tracked profiles for Nano and Pro Micro clone targets.
- [ ] 2.3 Add compile-time profile validation checks that fail fast for unsupported capabilities.

## 3. Transport and Build Selection

- [ ] 3.1 Implement platform transport adapters for serial MIDI and USB MIDI backends behind the shared MIDI interface.
- [ ] 3.2 Add centralized `build_config.h` selectors for board, wiring, and mapping profiles.
- [ ] 3.3 Add build-target definitions (Arduino CLI and/or PlatformIO environments) so switching targets requires no source edits.

## 4. Pluggable Mapping and Local Overrides

- [ ] 4.1 Add default MIDI mapping profile definitions separated from core behavior logic.
- [ ] 4.2 Add optional local override entrypoints (`build_config.local.h`, local mapping headers) and exclude them from version control.
- [ ] 4.3 Implement default fallback behavior when local override files are absent.

## 5. Validation and Documentation

- [ ] 5.1 Add dual-target compile checks for Nano and Pro Micro profiles.
- [ ] 5.2 Run hardware probe verification for divergent pins and document expected profile constraints.
- [ ] 5.3 Update README/developer docs with profile selection, mapping plugin workflow, and troubleshooting for alias mismatches.
