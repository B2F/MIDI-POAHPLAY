## ADDED Requirements

### Requirement: Build-time board and wiring profile selection
The firmware SHALL select exactly one board profile and one wiring profile at compile time through a centralized configuration boundary, without requiring source edits to switch targets.

#### Scenario: Select Nano default profile
- **WHEN** the build is invoked with Nano profile selectors
- **THEN** the resulting firmware SHALL compile with Nano pin assignments and configured transport backend

#### Scenario: Select Pro Micro clone profile
- **WHEN** the build is invoked with Pro Micro profile selectors
- **THEN** the resulting firmware SHALL compile with Pro Micro pin assignments and configured transport backend

### Requirement: Shared core independence from board pin constants
Core musical behavior modules SHALL operate without direct references to board pin labels or transport-specific classes.

#### Scenario: Core modules compile with alternate profiles
- **WHEN** board and wiring profiles are switched between supported targets
- **THEN** core modules SHALL compile unchanged and receive hardware behavior only through interfaces

### Requirement: Profile capability validation
The profile system SHALL validate required capabilities for selected features at compile time and fail with explicit errors when a selected profile is incompatible.

#### Scenario: Incompatible profile is selected
- **WHEN** a build selects a wiring profile that requires unsupported analog or output capabilities
- **THEN** compilation SHALL fail with a clear diagnostic describing the missing capability
