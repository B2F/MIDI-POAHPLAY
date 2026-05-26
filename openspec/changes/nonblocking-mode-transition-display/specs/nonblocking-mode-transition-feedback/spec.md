## ADDED Requirements

### Requirement: Mode transitions SHALL be non-blocking
When the active mode changes, the firmware MUST continue processing pads, encoders, faders, and scheduled note events without waiting on animated mode-label rendering.

#### Scenario: Mode switch during active pad performance
- **WHEN** a mode switch edge is detected while at least one pad is pressed
- **THEN** control processing SHALL continue in the same loop cycle without blocking on a scrolling mode animation

#### Scenario: Mode switch during encoder or fader movement
- **WHEN** a mode switch edge is detected while an encoder or fader is actively changing
- **THEN** the control update path SHALL remain responsive and SHALL NOT be delayed by transition-label animation

### Requirement: Mode transition feedback SHALL prioritize interaction continuity
The firmware SHALL provide mode feedback in a non-blocking form and MUST allow interaction-priority suppression when user input is active.

#### Scenario: Idle mode change feedback
- **WHEN** the mode changes and there is no active pad, encoder push, or control movement
- **THEN** the display SHALL show a concise static mode label without scrolling animation

#### Scenario: Active interaction during mode change
- **WHEN** the mode changes while user interaction is active
- **THEN** transition label rendering MAY be skipped for that cycle so interaction updates are not delayed

### Requirement: Reset safety behavior SHALL be preserved
The non-blocking mode feedback change MUST NOT alter reset-trigger safety behavior.

#### Scenario: Reset threshold entry
- **WHEN** reset threshold is reached on its rising edge
- **THEN** panic note-off behavior and reinitialization SHALL execute as before
