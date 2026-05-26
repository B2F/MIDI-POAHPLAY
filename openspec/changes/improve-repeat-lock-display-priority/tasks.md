## 1. Input Edge and Gesture Refactor

- [x] 1.1 Add explicit rising-edge detection for left/right encoder pushes and expose edge flags to interaction handlers.
- [x] 1.2 Refactor PLAY-mode pad lock/unlock invocation to run only on encoder-push rising edges instead of continuous while-held calls.
- [x] 1.3 Add REPEAT-mode pad repeat-lock gestures that mirror PLAY semantics: left push + pad unlocks, right push + pad locks.

## 2. Display Priority and Non-Blocking Feedback

- [x] 2.1 Introduce display-priority gating so any encoder-function message suppresses pad-note label writes while active.
- [x] 2.2 Implement non-blocking lock feedback messages with full wording (`Lock Pn`, `Unlock Pn`) and queue per affected pad.
- [x] 2.3 Ensure lock/unlock display messages are cancellable by higher-priority interaction updates without blocking loop timing.

## 3. Safety, Regression, and Documentation

- [x] 3.1 Gate note-layout resync calls so they run only when lock state actually changes, preventing retrigger buzz.
- [x] 3.2 Verify no regressions in repeat controls: left-push swing edit, right-push BPM edit, and no-push arp/rate controls.
- [x] 3.3 Update README Repeat/Play workflow documentation to include both-push repeat-lock gesture and encoder-message display priority behavior.
