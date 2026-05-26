## Context

Mode transitions are selected by switch edges and currently call `showModeIfChanged`, which uses a scrolling display animation. The scrolling call delays normal loop processing at the moment of mode changes, reducing responsiveness for pads, encoders, and faders during performance.

## Goals / Non-Goals

**Goals:**
- Keep mode transitions visually understandable without blocking control processing.
- Prioritize input handling over transition-label rendering when user interaction is active.
- Preserve existing reset behavior (`panicAllNotesOff` and `reinit`) and existing per-mode control mapping.

**Non-Goals:**
- Redesign mode architecture or switch semantics.
- Change CC/repeat/ultrasonic/play control assignments.
- Introduce new display hardware abstractions.

## Decisions

1. Replace scrolling transition text with immediate static labels.
   - Rationale: static `print` avoids long display animation calls and keeps loop latency low.
   - Alternative considered: shorten scroll duration. Rejected because it still introduces avoidable blocking.

2. Add interaction-priority suppression for transition labels.
   - Rationale: if pads are pressed or controls are moving during mode switch, showing labels is less important than uninterrupted control updates.
   - Alternative considered: always show labels. Rejected because it can still degrade responsiveness in dense live gestures.

3. Keep reset safety path unchanged.
   - Rationale: reset panic is a safety mechanism and must remain deterministic.
   - Alternative considered: special-case reset label behavior. Rejected to minimize risk in safety path.

## Risks / Trade-offs

- [Reduced mode visibility] Users may miss a verbose transition message. -> Mitigation: keep short static mode labels and allow normal parameter feedback immediately after.
- [Behavioral surprise] Existing users may expect scrolling animation. -> Mitigation: document new non-blocking mode feedback in README.
- [Interaction false positives] Overly broad activity detection could hide labels too often. -> Mitigation: use conservative interaction checks (active push or actual control delta).

## Migration Plan

1. Update mode label rendering to non-scrolling static output.
2. Introduce/adjust interaction check in mode transition display path.
3. Verify on-device responsiveness while switching modes during active play.
4. Rollback path: restore previous `scrollingText` call if regressions appear.

## Open Questions

- Should idle-only mode labels persist for a fixed short timeout, or only for a single loop update?
