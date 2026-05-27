## Context

Current firmware behavior includes a dedicated reset mode, which adds state complexity and a separate user journey before factory reset can happen. The requested change replaces this with a direct input gesture: double-pushing the encoder to trigger factory settings reset. The firmware already has encoder press handling and timing-sensitive input logic, so this change should be integrated into that existing event path with clear safety guards against accidental activation.

## Goals / Non-Goals

**Goals:**
- Remove reset mode entry behavior from runtime interaction.
- Detect a double-encoder push within a bounded timing window.
- Trigger the existing factory reset action directly from the validated gesture.
- Provide clear user feedback so users understand reset was triggered.

**Non-Goals:**
- Redesign all encoder interaction semantics beyond reset flow.
- Change the internal contents of factory defaults.
- Introduce additional hardware controls or external dependencies.

## Decisions

- Use a time-window double-press detector in encoder input handling.
  - Rationale: This keeps logic localized to where press events already exist and avoids adding another state machine mode.
  - Alternative considered: Long-press reset. Rejected because long press can conflict with existing hold interactions and is harder to distinguish in motion-heavy use.
- Remove reset mode state transitions entirely and route successful gesture directly to reset routine.
  - Rationale: Eliminates the mode that the user explicitly wants removed and reduces branching complexity.
  - Alternative considered: Keep reset mode as fallback. Rejected because it preserves user confusion and duplicate behavior.
- Guard reset activation with explicit double-press recognition and reset of detector state after timeout/trigger.
  - Rationale: Prevents accidental re-triggering and ensures normal single press behavior remains unchanged.
  - Alternative considered: Triple-press gesture. Rejected for lower discoverability and slower emergency use.

## Risks / Trade-offs

- False positives from noisy button events -> Add debounce and require distinct press events inside a strict interval.
- Users unaware of changed interaction -> Update on-device message path and project docs to call out double-press reset.
- Timing window too strict or too loose -> Start with a conservative window and validate through quick manual interaction tests.
- Removing mode may impact hidden code assumptions -> Audit reset-mode references and replace with direct call path to avoid dead branches.

## Migration Plan

- Implement gesture detection and direct reset trigger in firmware input path.
- Remove reset mode branches and associated prompts.
- Validate single press, double press, and timeout behavior on target hardware.
- If regressions occur, rollback by restoring prior reset-mode handler commit.

## Open Questions

- Exact double-press interval threshold in milliseconds should match user ergonomics on device.
