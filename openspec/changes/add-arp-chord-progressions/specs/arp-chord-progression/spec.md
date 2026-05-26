## ADDED Requirements

### Requirement: Progression playback in arp mode
The system SHALL support an optional chord progression layer in REPEAT mode that selects the active chord quality per progression step while preserving existing arp note-slot traversal and timing behavior.

#### Scenario: Progression disabled uses legacy chord behavior
- **WHEN** REPEAT mode is active and chord progression is disabled
- **THEN** arp playback SHALL use the current single chord selection behavior identical to legacy firmware

#### Scenario: Progression enabled applies step chord
- **WHEN** REPEAT mode is active, progression is enabled, and an arp trigger occurs at progression step N
- **THEN** the emitted chord voicing SHALL use the chord index assigned to progression step N

### Requirement: On-device progression editing controls
The system SHALL provide on-device controls in REPEAT mode to edit progression step selection, assign chord quality for the selected step, and toggle progression enable/disable with immediate display feedback.

#### Scenario: Edit progression step chord assignment
- **WHEN** the user enters progression edit context and rotates the chord assignment encoder
- **THEN** the selected progression step SHALL update to the chosen chord index and display the updated chord label

#### Scenario: Toggle progression playback
- **WHEN** the user triggers progression enable/disable control in REPEAT mode
- **THEN** the system SHALL toggle progression state and display an explicit enabled/disabled label

### Requirement: Safe fallback and reset behavior
The system SHALL keep progression state deterministic across reset and DRUM usage.

#### Scenario: Reset restores progression defaults
- **WHEN** reset is triggered through the configured reset threshold path
- **THEN** progression SHALL be disabled, cursor reset to first step, and all progression steps initialized to the default NOTE chord index

#### Scenario: DRUM scale bypasses progression chord voicing
- **WHEN** DRUM scale is active during REPEAT mode playback
- **THEN** arp output SHALL follow DRUM playback mapping without applying progression chord intervals
