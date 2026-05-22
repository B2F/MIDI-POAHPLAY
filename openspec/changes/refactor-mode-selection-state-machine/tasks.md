## 1. Input State Machine Foundations

- [x] 1.1 Add explicit runtime mode enums for selected non-reset mode and active mode (`PLAY`, `CC`, `REP`, `ULT`, `RES`).
- [x] 1.2 Add switch edge-tracking state for `SW_CC`, `SW_REPEAT`, `SW_ULTRASONIC`, and `SW_PLAY` to detect rising-edge events.
- [x] 1.3 Implement mode-selection event handling so each rising edge sets selected mode immediately and last rising edge wins.
- [x] 1.4 Implement reset override detection when all four switches are ON and gate reset action to run once per `RES` entry.
- [x] 1.5 Implement exit behavior from `RES` to force active mode back to `PLAY`.

## 2. Mode Routing and Behavior Mapping

- [x] 2.1 Replace priority-based mode routing branches with active-mode-driven routing in the main loop.
- [x] 2.2 Route fader and non-push encoder behavior so `CC` mode uses CC value control and non-CC modes use velocity/octave behavior.
- [x] 2.3 Route push-encoder behavior by active mode: `PLAY` scale/chord, `REP` arp/speed, `ULT` ultrasonic CC/distance, `CC` lane-index CC interactions, `RES` MIDI channel/base note.
- [x] 2.4 Ensure overlap and release handling follow spec: releasing switches does not alter selected non-reset mode.

## 3. Display and Verification

- [x] 3.1 Add mode transition detection and display mode labels (`REP`, `CC`, `PLAY`, `ULT`, `RES`) on activation edges.
- [x] 3.2 Verify reset display and behavior consistency with one-shot reset-on-entry semantics.
- [x] 3.3 Run firmware build for target environment and resolve any compilation issues introduced by the refactor.
- [ ] 3.4 Perform hardware behavior validation for default and remapped wiring profiles, covering mode selection, overlap, release semantics, and control mappings.
