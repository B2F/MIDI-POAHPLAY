## Context

The firmware already supports arp timing, pad slot traversal, chord voicing, and repeat/lock workflows in REPEAT mode. Today harmonic content is controlled by a single `selectedChord` value, so users cannot program a multi-step progression without live manual chord changes during performance. The requested change introduces an on-device progression layer while preserving existing arp timing and slot behavior.

## Goals / Non-Goals

**Goals:**
- Add a progression state machine that selects chord quality per progression step during arp playback.
- Provide an on-device REPEAT-mode workflow to edit progression step index, step chord, and progression enable/disable.
- Keep legacy behavior intact when progression is disabled.
- Ensure reset/panic paths restore deterministic progression defaults.

**Non-Goals:**
- No song arranger, pattern storage across power cycles, or multi-bank preset system.
- No changes to MIDI transport protocol or external control API.
- No chord-generation algorithm changes beyond selecting existing chord tables per step.

## Decisions

- **Decision: Add progression as an overlay on existing arp output path**
  - Rationale: Reuses stable timing/clock/repeat logic and minimizes regressions in slot ordering modes.
  - Alternative considered: Separate progression arp engine; rejected due to duplicated timing and higher bug risk.

- **Decision: Represent progression as fixed-length step array of chord indexes**
  - Rationale: Maps directly to existing `selectedChord` domain and requires no new harmonic model.
  - Alternative considered: Functional harmony (ii/V/I tags + scale-degree transforms); rejected for v1 complexity.

- **Decision: Keep progression controls in REPEAT mode with push-modified editing**
  - Rationale: Progression belongs to arp context and avoids conflict with PLAY mode remaps.
  - Alternative considered: New mode switch; rejected because hardware inputs are already dense.

- **Decision: Disable progression voicing logic in DRUM scale path**
  - Rationale: DRUM uses direct pad-to-note mapping where chord progression semantics do not apply.
  - Alternative considered: Apply intervals to drum notes; rejected as musically inconsistent and noisy.

- **Decision: Default progression to disabled with all steps set to NOTE**
  - Rationale: Backward compatibility and predictable startup behavior.
  - Alternative considered: Enable by default with canned progression; rejected to avoid surprising existing users.

## Risks / Trade-offs

- **[Input overload in REPEAT mode]** → Mitigation: Keep free rotation behavior unchanged for arp type/rate and only enter progression edit with explicit push context plus distinct display labels.
- **[Unexpected note transitions on progression step boundary]** → Mitigation: Advance progression cursor on deterministic arp timing boundaries and rely on existing NoteOff scheduling.
- **[State complexity causing reset inconsistencies]** → Mitigation: Initialize and clear all progression fields in `reinit()` and panic cleanup paths.
- **[User confusion about progression OFF vs ON]** → Mitigation: Explicit display feedback (`PrOn`/`PrOF`) and README control map update.
