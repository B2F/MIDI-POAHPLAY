## Context

The firmware currently handles hardware-input cleanup inline in `powaplay.ino`, including raw fader acquisition and threshold-based stabilization. This makes noise mitigation difficult to evolve and mixes electrical-signal conditioning concerns with mode/state behavior. The project also targets multiple board/wiring profiles, so cleanup logic must remain pin-agnostic and reusable.

## Goals / Non-Goals

**Goals:**
- Isolate hardware cleanup responsibilities into a dedicated module/file under `src/`.
- Provide deterministic, low-latency cleanup primitives for noisy analog and digital inputs.
- Preserve existing behavior by default and allow incremental tuning via profile/config constants.
- Keep integration boundaries simple so existing app loop and mode logic remain intact.

**Non-Goals:**
- Redesign mode-selection/state-machine behavior.
- Change board profile pin mappings or transport behavior.
- Introduce external libraries or dynamic allocation.

## Decisions

1. Create a dedicated hardware cleanup module with a small API.
   - Decision: Add a new module (for example `src/hal/hardware_cleanup.h/.cpp`) that exposes stateless helpers and lightweight per-input state structs for filtering/debounce.
   - Rationale: Keeps `powaplay.ino` focused on musical behavior while centralizing signal-conditioning logic.
   - Alternative considered: Keep helper functions in `powaplay.ino`; rejected because it preserves coupling and hampers reuse.

2. Use layered cleanup for analog controls.
   - Decision: Support median-of-3 spike rejection followed by EMA smoothing and final hysteresis/deadband.
   - Rationale: Median handles transient spikes; EMA stabilizes continuous noise; hysteresis prevents chattering around boundaries.
   - Alternative considered: Threshold-only filtering; rejected because noisy wiring can still cross threshold repeatedly.

3. Keep configuration compile-time and profile-friendly.
   - Decision: Add tunable constants in mapping/profile configuration and consume them in the cleanup module.
   - Rationale: Different boards and wiring quality need different filter strengths without touching core logic.
   - Alternative considered: Hardcode constants in module; rejected due to poor portability.

4. Preserve responsiveness with microsecond-level overhead only.
   - Decision: Avoid blocking delays except optional tiny settling points where needed and enforce optional minimum CC send interval at millisecond scale.
   - Rationale: Maintain performance feel for faders/encoders while reducing MIDI spam.
   - Alternative considered: Heavy averaging windows; rejected due to added control lag.

## Risks / Trade-offs

- [Over-filtering can feel sluggish] -> Mitigation: conservative defaults and per-profile tuning values.
- [Under-filtering still jitters on very noisy builds] -> Mitigation: expose deadband/hysteresis/EMA knobs and document tuning steps.
- [Refactor may alter edge behavior] -> Mitigation: preserve existing thresholds as baseline and add targeted regression checks for unchanged paths.
- [Extra code paths increase complexity] -> Mitigation: keep API minimal, deterministic, and well-scoped to input cleanup only.

## Migration Plan

1. Introduce new cleanup module and compile it without changing call sites.
2. Move fader read/conditioning path behind the module API.
3. Move debounce or stabilization helpers for switch/button reads as follow-up within same change where applicable.
4. Verify functional parity on existing profiles and tune defaults for noisy Nano wiring.
5. If issues appear, rollback by routing call sites back to prior inline functions while keeping module files present.

## Open Questions

- Should fader CC deadband be global or per lane?
- Should encoder-related cleanup be included now or kept out since encoder A/B is already direct-read?
- Do we need profile-specific defaults for Nano vs Pro Micro at merge time or can one default set cover both?
