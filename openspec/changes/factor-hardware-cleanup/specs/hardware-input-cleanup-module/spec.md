## ADDED Requirements

### Requirement: Dedicated hardware cleanup module
The firmware SHALL provide a dedicated hardware cleanup module in a separate source file that encapsulates input-conditioning behavior used by the application loop.

#### Scenario: Main loop consumes cleanup module
- **WHEN** the firmware is built with existing board and wiring profiles
- **THEN** `powaplay.ino` SHALL call the cleanup module API for hardware input conditioning instead of duplicating equivalent inline cleanup logic.

### Requirement: Stable analog control conditioning
The cleanup module SHALL provide analog-conditioning behavior that reduces jitter on noisy controls while preserving performer responsiveness.

#### Scenario: Noisy fader at rest
- **WHEN** a fader input fluctuates within configured noise bounds while the physical control is not intentionally moved
- **THEN** the conditioned value SHALL remain stable and SHALL NOT produce continuous MIDI CC churn.

#### Scenario: Intentional fader movement
- **WHEN** a performer moves a fader beyond configured hysteresis/deadband thresholds
- **THEN** the conditioned value SHALL track movement smoothly and SHALL update output without perceptible lag.

### Requirement: Configurable cleanup tuning
The cleanup module SHALL support compile-time tuning parameters for filtering, hysteresis/debounce, and output deadband through project profile/config definitions.

#### Scenario: Profile-specific tuning
- **WHEN** a profile changes cleanup constants for a given board or wiring quality
- **THEN** the firmware SHALL apply those values without requiring core logic edits in `powaplay.ino`.

### Requirement: Backward-compatible default behavior
Default cleanup configuration SHALL preserve existing control semantics unless explicitly tuned.

#### Scenario: Existing default mapping unchanged
- **WHEN** no new tuning overrides are enabled beyond default values
- **THEN** the firmware SHALL maintain existing functional control behavior while improving noise robustness.
