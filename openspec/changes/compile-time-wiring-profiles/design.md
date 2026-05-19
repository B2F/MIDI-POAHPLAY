## Context

The current firmware is a large sketch where MIDI behavior, pin assignments, and board assumptions are tightly coupled. This makes support for Nano and generic Pro Micro clones brittle, especially for divergent pins, serial-vs-USB transport, and local custom wiring needs.

## Goals / Non-Goals

**Goals:**
- Separate musical behavior from hardware and transport concerns so both board targets share one core.
- Replace scattered conditional compilation with a centralized build configuration boundary.
- Introduce wiring and MIDI mapping plugins that can be selected at compile time.
- Allow non-versioned local mapping/wiring overrides for end users while keeping repository defaults stable.

**Non-Goals:**
- Runtime board auto-detection.
- Supporting every third-party board package alias combination without profile validation.
- Immediate full rewrite of all legacy logic before preserving behavior parity.

## Decisions

- Use a layered architecture (`core`, `hal`, `platform`, `profiles`, `config`) with a thin `.ino` orchestration layer.
  - Rationale: isolates behavior from hardware and limits where compile-time selection is handled.

- Use typed board/wiring profile structs plus capability flags.
  - Rationale: profile data is explicit, testable, and easier to extend than preprocessor branches.
  - Alternative considered: keep direct `#if` blocks in app logic; rejected due to maintainability risk.

- Use compile-time backend selection for MIDI transport via build flags.
  - Rationale: minimizes binary size and avoids runtime dispatch overhead on AVR.
  - Alternative considered: runtime polymorphic transport plugin; rejected due to avoidable complexity/RAM usage.

- Add local override entrypoints (`build_config.local.h`, optional local mapping headers) excluded from version control.
  - Rationale: users can customize wiring/mapping without polluting tracked profiles.

- Keep a dual-target build matrix as a regression gate.
  - Rationale: prevents Nano behavior regressions while evolving Pro Micro support.

## Risks / Trade-offs

- [Profile/schema overdesign slows migration] -> Mitigation: start with minimal required fields and expand only when needed.
- [Alias mismatch across board cores (A6/A7, D11-D13)] -> Mitigation: include capability flags and probe checklist per profile.
- [Legacy behavior drifts during extraction] -> Mitigation: preserve baseline tests and migrate in thin slices with parity checks.
- [User local overrides break silently] -> Mitigation: add compile-time validation and clear fallback to default profiles.

## Migration Plan

1. Extract MIDI send/read calls behind `midi_iface` while keeping existing behavior.
2. Move pin constants into default Nano and Pro Micro profile headers.
3. Introduce build-time selectors in `build_config.h` and map to one transport/profile combo.
4. Add local override include pattern and gitignore entries for user-specific mappings.
5. Refactor loop/setup orchestration to call `core` + `hal` interfaces only.
6. Validate both targets in compile matrix and hardware probe checklist.

## Open Questions

- Should PlatformIO be the primary build entrypoint, or remain Arduino CLI first with optional PlatformIO?
- Do we ship one official Pro Micro wiring profile or two (minimal-rewire and clone-safe) at initial release?
