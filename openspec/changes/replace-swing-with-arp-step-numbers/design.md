## Context

The sequencing flow currently uses a swing parameter to alter playback feel. The requested change replaces swing with arp step numbers so users can set deterministic step timing behavior directly in arpeggiator steps. This touches multiple modules: UI controls, state/model schema, playback timing logic, and project serialization.

## Goals / Non-Goals

**Goals:**
- Replace swing as an active feature in the arp sequencing path.
- Introduce per-step arp numeric values that are editable, validated, and persisted.
- Ensure playback engine behavior is fully driven by arp step numbers.
- Provide backward-compatible loading behavior for existing projects.

**Non-Goals:**
- Redesign the full sequencer UX beyond replacing swing controls.
- Introduce new transport timing systems outside arp step evaluation.
- Add external dependencies or new storage backends.

## Decisions

1. Replace swing field with `arpStepNumber` (or equivalent existing naming style) at step granularity.
   - Rationale: The feature intent is step-level control, not global groove offset.
   - Alternative considered: keep both swing and step numbers; rejected because it creates conflicting timing authorities.

2. Migrate legacy data on read instead of requiring a hard format bump.
   - Rationale: Keeps older projects loadable without manual conversion.
   - Alternative considered: fail old payloads; rejected due to poor user experience and unnecessary breakage.

3. Centralize timing interpretation in playback engine using normalized step-number rules.
   - Rationale: A single interpretation path avoids UI/playback drift and simplifies testing.
   - Alternative considered: derive timing partly in UI model; rejected because duplicated logic increases inconsistency risk.

4. Remove swing UI affordances and replace with arp step number controls in the same interaction zone.
   - Rationale: Reduces user confusion and keeps workflow continuity.
   - Alternative considered: hide swing but keep data writable through old API fields; rejected because hidden state is hard to reason about.

## Risks / Trade-offs

- Legacy projects with swing-only values may map imperfectly to step numbers -> Mitigation: define deterministic defaults and migration rules, and document behavior.
- Removing swing can break integrations expecting swing fields -> Mitigation: maintain temporary read compatibility and deprecation notes for serialized shape.
- Step-number validation bugs can cause silent timing errors -> Mitigation: enforce explicit bounds and add unit tests for invalid, min, max, and edge transition cases.
- Playback feel may change for users accustomed to swing -> Mitigation: choose sensible default step numbers and provide clear UI labels/tooltips.
