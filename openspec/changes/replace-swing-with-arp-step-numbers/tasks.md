## 1. Data Model

- [x] 1.1 Identify and remove swing fields from arp step state/model structures.
- [x] 1.2 Set arp step number defaults and bounds to `2..16` with default `8`.

## 2. Playback Engine Updates

- [x] 2.1 Replace timing-multiplier interpretation with arp-cycle step-count traversal.
- [x] 2.2 Apply octave rollover for step values above 8 by continuing an octave-up pass before restart.
- [x] 2.3 Keep arp rate/speed as timing control and use step number only for note progression.

## 3. UI and Interaction Changes

- [x] 3.1 Remove swing controls from arp-related editor views.
- [x] 3.2 Wire left-push repeat edit to arp step number display/control (`St2..St16`).
- [x] 3.3 Ensure step-number edits immediately affect subsequent arp note selection.
- [x] 3.4 Keep NOTE arp behavior unchanged when step number is edited.

## 4. Verification and Cleanup

- [ ] 4.1 Manual device verification: step `2`, `8`, `9`, and `16` produce expected traversal and octave rollover.
- [x] 4.4 Remove dead swing-related code paths and update any relevant developer docs.
