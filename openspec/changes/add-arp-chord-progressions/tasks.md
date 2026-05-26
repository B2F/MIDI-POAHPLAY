## 1. Progression state model

- [ ] 1.1 Add progression state fields (enabled flag, step cursor, selected edit step, fixed step array of chord indexes) with clear defaults.
- [ ] 1.2 Initialize and reset all progression fields in setup and `reinit()` so reset behavior is deterministic.

## 2. REPEAT-mode controls and display

- [ ] 2.1 Add REPEAT-mode progression edit input handling for step selection and per-step chord assignment without breaking existing arp type/rate free-rotation controls.
- [ ] 2.2 Add progression enable/disable toggle control in REPEAT mode with explicit display labels (enabled/disabled).
- [ ] 2.3 Add display feedback for current progression edit step and assigned chord label updates.

## 3. Arp playback integration

- [ ] 3.1 Integrate progression chord resolution into arp trigger path so step chord is used when progression is enabled.
- [ ] 3.2 Keep legacy single-chord playback path unchanged when progression is disabled.
- [ ] 3.3 Advance progression cursor on deterministic arp timing boundaries and wrap at progression length.

## 4. Safety paths and mode exceptions

- [ ] 4.1 Bypass progression chord voicing when DRUM scale is active so DRUM mapping remains authoritative.
- [ ] 4.2 Ensure panic/reset and latch/arp state cleanup does not leave stale progression cursor or inconsistent step state.

## 5. Verification and documentation

- [ ] 5.1 Validate key scenarios: progression OFF legacy behavior, progression ON step chord changes, reset defaults restored, and DRUM bypass behavior.
- [ ] 5.2 Update README REPEAT-mode controls and progression usage notes to match implemented behavior.
