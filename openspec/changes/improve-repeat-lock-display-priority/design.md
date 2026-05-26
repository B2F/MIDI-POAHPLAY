## Context

The firmware currently mixes real-time note triggering, mode control, and display updates inside a single loop. In PLAY mode, lock/unlock handling is called continuously while encoder push is held, which repeatedly resynchronizes held notes and can create audible buzzing. In REPEAT mode, right push was remapped to BPM edit and left push to swing edit, removing the previous repeat-lock path unless a new gesture is introduced. Display output currently allows lower-priority pad-note labels to overwrite interaction-critical encoder feedback.

The solution must preserve low-latency behavior, avoid blocking display calls, and not regress arp timing or note-off safety.

## Goals / Non-Goals

**Goals:**
- Make PLAY lock/unlock actions edge-triggered and idempotent per push gesture.
- Provide non-blocking lock feedback using full words (`Lock`, `Unlock`) with pad identity.
- Add a deterministic repeat-lock toggle gesture: both encoder pushes + pad press.
- Enforce display priority so encoder-driven messages always take precedence over pad-note labels.
- Preserve existing control mappings: no-push repeat controls, left-push swing edit, right-push BPM edit.

**Non-Goals:**
- Redesigning mode architecture or display subsystem APIs beyond required priority hooks.
- Changing repeat timing math, gate model, or MIDI sync behavior in this change.
- Adding new persistence/storage for lock states across resets.

## Decisions

- **Edge-trigger lock/unlock in PLAY mode**
  - Decision: Trigger PLAY lock and unlock workflows on encoder push rising edges, not continuously while held.
  - Rationale: Prevent repeated note retrigger/sync calls that produce buzzing while retaining expected user intent.
  - Alternative considered: Keep while-held and debounce with timer; rejected due to hidden latency and still-coupled behavior.

- **Both-push pad gesture for repeat lock toggle**
  - Decision: In REPEAT mode, detect both pushes active and toggle `repeatIsLocked[pad]` on pad press edge.
  - Rationale: Restores repeat-lock accessibility without colliding with BPM/swing encoder-edit gestures.
  - Alternative considered: Long-press or modal lock arm; rejected due to ambiguity and higher performance friction.

- **Encoder-message display priority arbitration**
  - Decision: Introduce a loop-level priority gate that suppresses pad-note label updates when any encoder function is actively presenting a message.
  - Rationale: Makes UX deterministic and prevents pad-note text from overwriting control feedback.
  - Alternative considered: Last-write-wins display policy; rejected because it is non-deterministic under concurrent input.

- **Non-blocking lock/unlock feedback format**
  - Decision: Queue non-blocking messages with full words and pad index (`Lock Pn`, `Unlock Pn`), one message per affected pad.
  - Rationale: Matches requested wording while preserving responsive loop behavior.
  - Alternative considered: abbreviated labels; rejected by requirement.

## Risks / Trade-offs

- **[Input edge complexity]** More explicit edge tracking can introduce missed transitions if update order is wrong. -> Mitigation: compute push and pad edges once per loop before action handlers; keep deterministic ordering.
- **[Display queue contention]** Multiple lock messages for multi-pad actions may delay subsequent informational messages. -> Mitigation: make lock messages cancellable by higher-priority interaction updates.
- **[Gesture discoverability]** Both-push repeat toggle is powerful but less obvious than single-push behavior. -> Mitigation: document in README mode table and workflow notes.
- **[Behavior drift in repeat mode]** New toggle path could accidentally affect swing/BPM edits. -> Mitigation: gate toggle strictly on pad press edge while both pushes are active; keep encoder-turn handlers unchanged.
