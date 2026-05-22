## Context

Current mode behavior has evolved through incremental fixes and now mixes raw switch interpretation, mode-priority arbitration, and legacy routing expectations in one runtime path. Custom profile remapping exposed unstable behavior where modes could invert, stick, or fail to activate. The change must preserve existing musical feature logic while replacing mode selection semantics with deterministic event-driven behavior.

Constraints:
- Firmware runs on ATmega328P and must remain lightweight.
- Existing pad/arp/midi logic should not be rewritten unless required.
- Encoder push polarity and mode switch polarity may differ and must remain independently configurable.

## Goals / Non-Goals

**Goals:**
- Define a deterministic mode state machine with explicit event semantics.
- Select CC/REP/ULT/PLAY modes on switch rising edges; last ON event wins under overlap.
- Enter RES mode only when all four mode switches are ON, execute reset exactly once per RES entry, and return to PLAY when RES exits.
- Drive fader and encoder behavior from explicit active mode mapping instead of implicit priority arbitration.
- Display short mode labels (`REP`, `CC`, `PLAY`, `ULT`, `RES`) immediately on mode activation.

**Non-Goals:**
- Redesigning pad arp algorithms, chord tables, scale tables, or MIDI transport behavior.
- Introducing compatibility shims that preserve old mode-priority behavior.
- Changing hardware pin maps or board profiles.

## Decisions

1. **Adopt an event-driven selected mode model**
   - Maintain `selectedMode` for non-reset runtime modes (`PLAY`, `CC`, `REP`, `ULT`).
   - On rising edge of each mode switch, set `selectedMode` to that mode.
   - Rationale: removes continuous-priority contention and aligns with user expectation that pressing ON selects mode immediately.
   - Alternative considered: keep continuous arbitration with revised priority. Rejected due to repeated overlap regressions.

2. **Make reset an override mode, not a latched mode**
   - Compute `activeMode = RES` only when all four switches are ON.
   - Trigger reset behavior once on entry edge into RES.
   - On RES exit, force `activeMode` to PLAY (per accepted behavior decision).
   - Alternative considered: return to previously selected mode. Rejected by explicit user direction.

3. **Separate input decoding from mode transitions**
   - Keep input normalization (`currentInputState`) as raw logical switch state.
   - Add explicit edge tracking for mode switches (`prev` state per switch) and derive transition events from normalized state.
   - Rationale: avoids reintroducing raw pin coupling and keeps mode logic testable.

4. **Route controls by active mode table**
   - Fader/non-push behavior:
     - PLAY: velocity + octave
     - CC: fader CC values
     - REP/ULT/RES: use non-CC behavior
   - Encoder push behavior:
     - PLAY: scale/chord
     - REP: arp/speed
     - ULT: ultrasonic CC/distance
     - CC: lane-mapped CC interactions by fader index order
     - RES: MIDI channel/base note
   - Rationale: explicit behavior mapping prevents legacy boundary leaks.

5. **Emit mode label only on mode transition**
   - Show labels at activation edge, not continuously.
   - Rationale: keeps display stable and avoids flicker while switches remain held.

## Risks / Trade-offs

- **[Risk]** Rising-edge detection may misfire due to noisy channels.  
  **Mitigation:** reuse stabilized mux reads and keep edge detection based on normalized per-loop state.

- **[Risk]** Behavior changes for users accustomed to old simultaneous-switch arbitration.  
  **Mitigation:** enforce documented rule set and explicit labels so behavior is predictable.

- **[Risk]** Reset override (all four ON) could be accidentally entered during overlap experiments.  
  **Mitigation:** one-shot reset on RES entry and immediate PLAY fallback on exit reduce repeated destructive actions.

- **[Trade-off]** No compatibility path to old mode-priority flow.  
  **Mitigation:** accepted explicitly to remove legacy ambiguity and simplify mental model.
