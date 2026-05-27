## 1. Input Gesture Detection

- [x] 1.1 Locate encoder push handling path and add double-press timing state.
- [x] 1.2 Implement double-encoder push detection using a bounded interval and existing debounce behavior.
- [x] 1.3 Reset gesture detector state on timeout and after successful trigger to prevent repeated accidental resets.

## 2. Reset Flow Refactor

- [x] 2.1 Remove reset mode entry/state transitions from firmware interaction flow.
- [x] 2.2 Route successful double-press gesture directly to the existing factory reset routine.
- [x] 2.3 Remove or update any code paths that assume reset mode still exists.

## 3. Feedback and Validation

- [x] 3.1 Update reset confirmation messaging/feedback text to describe double-encoder push behavior.
- [x] 3.2 Manually validate single press, in-window double press, and out-of-window double press behaviors on device.
- [x] 3.3 Update documentation references that mention reset mode to the new factory reset gesture.
