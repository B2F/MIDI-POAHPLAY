## ADDED Requirements

### Requirement: Pluggable mapping profile selection
The firmware SHALL support compile-time selection of MIDI mapping profiles independent of board profiles.

#### Scenario: Select alternate mapping profile
- **WHEN** the build selects a non-default mapping profile
- **THEN** control-to-MIDI assignments SHALL follow the selected profile without changing core logic files

### Requirement: Non-versioned local mapping overrides
The project SHALL provide an optional local configuration entrypoint for user-defined mapping/wiring profiles that is excluded from version control.

#### Scenario: Use local mapping override
- **WHEN** a local override file is present and selected by build configuration
- **THEN** the build SHALL use the local mapping profile while repository default profiles remain unchanged

### Requirement: Safe fallback behavior for missing local overrides
The build configuration SHALL fall back to repository-default mapping and wiring profiles when local override files are absent.

#### Scenario: Local override not present
- **WHEN** local override paths are not provided or files are missing
- **THEN** the build SHALL compile using default tracked profiles without manual source modifications
