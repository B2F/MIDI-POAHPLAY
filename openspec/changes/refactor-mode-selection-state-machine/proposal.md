## Why

Mode detection has regressed repeatedly under custom wiring: modes can stick, invert, or fail to activate because runtime behavior is still coupled to raw switch state assumptions. A deterministic mode state machine is needed now to unblock testing and make mode behavior predictable across default and remapped profiles.

## What Changes

- Add an explicit runtime mode state machine that selects active mode from switch ON events instead of continuous priority arbitration.
- Make mode selection event-driven: CC/REPEAT/ULTRASONIC/PLAY are selected on their switch rising edge; last rising edge wins when switches overlap.
- Add a reset override mode entered only when all four mode switches are ON simultaneously, triggering reset-on-entry behavior once.
- Standardize mode activation display labels to `REP`, `CC`, `PLAY`, `ULT`, and `RES`.
- Route encoder-push and fader/non-push control behavior from active runtime mode definitions instead of implicit legacy boundaries.
- Remove remaining compatibility mode-priority logic from operational behavior.

## Capabilities

### New Capabilities
- `mode-selection-state-machine`: Defines deterministic mode transitions, overlap handling, reset override behavior, and mode-specific control/display mappings.

### Modified Capabilities


## Impact

- Affected code: `powaplay.ino` mode input decode, mode transition logic, encoder/fader routing, and mode display update paths.
- Affected configuration: build-time input polarity selectors in `src/config/build_config.h` remain supported but runtime mode selection no longer depends on fixed priority arbitration.
- No external API changes.
