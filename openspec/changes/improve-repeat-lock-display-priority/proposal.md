## Why

Pad lock/unlock actions in PLAY mode currently retrigger continuously while encoder pushes are held, which can cause audible buzzing and unstable feedback during performance. Recent Repeat-mode encoder remaps also made repeat lock harder to access and exposed display-priority conflicts where pad-note labels can overwrite more important encoder feedback.

## What Changes

- Make PLAY-mode pad lock/unlock actions one-shot on encoder-push edge instead of continuous while-held execution.
- Move lock feedback to non-blocking display messages using full words and pad context (`Lock Pn`, `Unlock Pn`).
- Add Repeat-mode repeat-lock toggle on `both encoder pushes + pad press` so repeat lock remains accessible while preserving left-push swing and right-push BPM editing.
- Enforce a display-priority policy where encoder-function messages always win over pad-note labels when both are active.
- Keep arp timing and note-off safety behavior intact while preventing retrigger storms during lock workflows.

## Capabilities

### New Capabilities
- `encoder-priority-lock-feedback`: Non-blocking lock/unlock UX with deterministic input gestures and encoder-message display priority over pad-note display.

### Modified Capabilities
- None.

## Impact

- Firmware interaction logic in `powaplay.ino` for push gesture handling and lock/repeat-lock transitions.
- Display arbitration and non-blocking message flow under simultaneous pad and encoder activity.
- Repeat-mode gesture map and usability during live performance.
