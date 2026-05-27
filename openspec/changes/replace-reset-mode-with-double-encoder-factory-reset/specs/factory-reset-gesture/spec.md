## ADDED Requirements

### Requirement: Double-encoder push triggers factory reset
The system SHALL trigger a factory settings reset when the encoder push button is pressed twice within the configured double-press interval.

#### Scenario: Double press within interval resets device
- **WHEN** the user performs two valid encoder push presses within the double-press interval
- **THEN** the system triggers factory settings reset immediately

#### Scenario: Presses outside interval do not reset
- **WHEN** the user performs a second encoder push press after the double-press interval expires
- **THEN** the system treats the second press as a new first press and does not trigger factory reset

### Requirement: Reset mode interaction is removed
The system MUST NOT require or expose a dedicated reset mode as part of factory reset initiation.

#### Scenario: Reset mode entry path is unavailable
- **WHEN** the user performs interactions that previously entered reset mode
- **THEN** the system does not enter reset mode and continues normal interaction handling

### Requirement: Reset trigger provides confirmation feedback
The system SHALL provide explicit user feedback when factory reset is triggered by the double-encoder push gesture.

#### Scenario: Feedback appears after successful gesture
- **WHEN** factory reset is triggered by a valid double push
- **THEN** the system emits reset confirmation feedback through existing device feedback channels
