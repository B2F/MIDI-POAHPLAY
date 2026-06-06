## Context

The current firmware has compile-time board, wiring, mapping, and MIDI transport selection, but the supported targets are still AVR-class boards with limited flash and RAM. A velocity-aware step sequencer needs enough memory for a 64-step grid and enough flash for new mode handling, edit UI, recording logic, and playback scheduling. Teensy 4.0 provides the required headroom and native USB MIDI while moving the hardware design to a 3.3 V logic domain.

The existing mode system has four operating modes selected by physical switches. The new STEP mode adds a fifth switch and an 8-LED visible grid page for sequencer builds. Existing profiles must remain valid without declaring those signals when the sequencer feature is disabled.

## Goals / Non-Goals

**Goals:**

- Add an optional STEP mode that is compiled only when explicitly enabled.
- Add Teensy 4.0 as the primary hardware profile for the expanded sequencer build.
- Preserve existing Nano and Pro Micro behavior and profile validity when sequencer support is disabled.
- Store step data for up to 64 grid positions: 1:16 resolution across 4 bars.
- Store velocity per recorded note/grid item, using live velocity during live recording and current velocity when a note is added in grid mode.
- Keep fader behavior consistent by configuring analog reads to the existing 10-bit range.
- Require the expanded hardware profile to use 3.3 V-safe ultrasonic input behavior.

**Non-Goals:**

- Persisting sequencer patterns across power cycles.
- Multiple pattern banks or song chaining.
- Per-note microtiming, probability, ratchets, ties, or swing.
- Supporting the step sequencer on AVR profiles by default.
- Replacing existing REPEAT/arp behavior.

## Decisions

- Use a build-time feature gate for sequencer inclusion.
  - Rationale: existing AVR builds are flash constrained, so the new mode must not be pulled into legacy targets accidentally.
  - Alternative considered: enable the sequencer whenever a wiring profile declares a STEP switch; rejected because hardware metadata should not implicitly change binary feature size.

- Add a separate opt-in step hardware profile block instead of appending STEP fields directly to every wiring profile initializer.
  - Rationale: legacy wiring headers remain source-compatible and cannot accidentally default new `SignalRef` fields to pin 0.
  - Alternative considered: add STEP switch and LED fields directly to `WiringProfile`; rejected because it would force edits to all existing profiles or risk unsafe default initialization.

- Treat one grid page as 8 grid items and require one step LED per visible item whenever STEP mode is enabled.
  - Rationale: the existing controller has 8 pads, so an 8-item page maps naturally to direct pad editing and visual feedback.
  - Alternative considered: use 4 LEDs as bar/page indicators; rejected after clarifying that LEDs represent visible grid items.

- Store the sequencer grid as a 64-step by 8-lane pattern with velocity per recorded note.
  - Rationale: maximum grid length is 1:16 over 4 bars, and each lane corresponds to one pad.
  - Alternative considered: one velocity per step; rejected because live recording and grid editing require velocity per recorded note.

- Keep STEP mode controls aligned with existing two-encoder workflows.
  - Left encoder selects the current lane.
  - Right encoder selects grid resolution among 1:4, 1:8, and 1:16.
  - Left encoder push edits bar count from 1 to 4.
  - Right encoder push selects live recording or grid note-selection recording.

- Configure Teensy analog reads to 10-bit resolution with `analogReadResolution(10)`.
  - Rationale: smallest compatible fix for existing fader thresholds and mapping assumptions.
  - Alternative considered: normalize every analog read in the HAL; useful long term, but larger than needed for this change.

- Require 3.3 V-compatible ultrasonic behavior in the Teensy profile.
  - Rationale: Teensy GPIO is not 5 V tolerant.
  - Alternative considered: allow classic HC-SR04 directly; rejected because its 5 V echo can damage 3.3 V inputs.

## Risks / Trade-offs

- [Sequencer UI overloads the two encoders and small display] -> Keep first version focused on lane, resolution, bars, recording mode, and 8-item page editing only.
- [Velocity per recorded note increases RAM versus per-step velocity] -> Use Teensy 4.0 as the sequencer target and keep AVR feature gate disabled by default.
- [Step hardware profile can complicate profile validation] -> Require an explicit step hardware opt-in block only when the sequencer feature is enabled; leave legacy profiles untouched when disabled.
- [Teensy build may require different library choices than AVR] -> Add a dedicated PlatformIO environment and keep AVR envs unchanged.
- [3.3 V migration can break sensor behavior] -> Document supported ultrasonic modules or require level shifting for any 5 V echo output.

## Migration Plan

1. Extend build configuration with a disabled-by-default step-sequencer feature flag.
2. Add Teensy 4.0 board/profile definitions and a Teensy build environment.
3. Add a separate step hardware profile schema with one STEP switch and 8 step-grid LED signals.
4. Add compile-time validation that sequencer builds require explicit step hardware support.
5. Configure Teensy setup to use `analogReadResolution(10)`.
6. Add STEP mode state, display label, grid data model, recording mode state, and playback scheduler.
7. Verify existing Nano and Pro Micro builds remain behaviorally unchanged with the feature disabled.
8. Verify Teensy STEP mode with the expanded hardware profile.

## Open Questions

- Should the sequencer play simultaneously with REPEAT/arp mode, or should STEP mode own sequenced playback while active?
- Should live recording quantize pad hits to the current grid immediately, or should it record only on exact clock ticks in the first version?
- Should live recording use the current global velocity/fader value only, or preserve any future per-pad velocity source if added later?
