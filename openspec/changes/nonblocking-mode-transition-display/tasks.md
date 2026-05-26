## 1. Transition Feedback Refactor

- [x] 1.1 Replace mode-change `scrollingText` usage with non-blocking static label rendering in `showModeIfChanged`.
- [x] 1.2 Keep mode label mapping concise for 4-digit display and verify each mode label remains distinguishable.

## 2. Interaction-Priority Behavior

- [x] 2.1 Add an activity check (pad pressed, encoder push, encoder delta, or fader delta) for mode-change cycles.
- [x] 2.2 Suppress mode label rendering for the current cycle when activity is detected so control updates stay responsive.

## 3. Safety and Regression Validation

- [x] 3.1 Verify reset threshold behavior still executes panic note-off and `reinit` unchanged.
- [x] 3.2 Validate mode switching under active play (pads/encoders/faders/repeat) does not stall control processing.
- [x] 3.3 Update README mode behavior notes to reflect non-blocking transition feedback.
