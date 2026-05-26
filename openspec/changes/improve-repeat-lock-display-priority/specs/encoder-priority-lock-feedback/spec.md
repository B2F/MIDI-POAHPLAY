## ADDED Requirements

### Requirement: Play lock actions are edge-triggered and non-repeating
The system SHALL apply PLAY-mode lock and unlock workflows only once per relevant encoder-push rising edge, and SHALL NOT continuously reapply lock state while the same encoder push remains held.

#### Scenario: Lock applies once while push is held
- **WHEN** the performer enters PLAY mode, holds left encoder push, and keeps it held across multiple loop iterations with one or more pads active
- **THEN** lock state transitions are applied once per target pad for that push edge and are not repeatedly re-triggered until the push is released and pressed again

#### Scenario: Unlock applies once while push is held
- **WHEN** the performer enters PLAY mode, holds right encoder push, and keeps it held across multiple loop iterations with one or more pads active
- **THEN** unlock state transitions are applied once per target pad for that push edge and are not repeatedly re-triggered until the push is released and pressed again

### Requirement: Lock feedback is non-blocking and uses full words
The system SHALL present lock workflow feedback using non-blocking display messaging and SHALL render full-word labels `Lock` and `Unlock` with pad identity.

#### Scenario: Single-pad lock feedback
- **WHEN** one pad is affected by a lock action
- **THEN** the display shows a non-blocking message formatted as `Lock Pn` for that pad

#### Scenario: Single-pad unlock feedback
- **WHEN** one pad is affected by an unlock action
- **THEN** the display shows a non-blocking message formatted as `Unlock Pn` for that pad

### Requirement: Repeat lock toggle is available with both-push pad gesture
In REPEAT mode, the system SHALL toggle a pad's repeat lock state when both encoder pushes are active and the pad receives a press rising edge.

#### Scenario: Toggle lock on both-push pad press
- **WHEN** REPEAT mode is active, both encoder pushes are held, and pad `Pn` is newly pressed
- **THEN** `Pn` repeat lock toggles between locked and unlocked and the system emits the corresponding `Lock Pn` or `Unlock Pn` non-blocking message

#### Scenario: No toggle without both pushes
- **WHEN** REPEAT mode is active and a pad is pressed without both encoder pushes active
- **THEN** repeat lock state for that pad does not toggle via this gesture path

### Requirement: Encoder-function messages have priority over pad-note labels
The system SHALL prioritize any encoder-function display message above pad-note label display when both are active in the same interaction window.

#### Scenario: Encoder message suppresses pad-note overwrite
- **WHEN** an encoder-function message is active and a pad-note label update is generated in the same loop window
- **THEN** the encoder-function message remains visible and the pad-note label update is suppressed

#### Scenario: Pad-note labels resume after encoder context ends
- **WHEN** encoder-function messaging is no longer active
- **THEN** subsequent pad-note label updates are allowed again
