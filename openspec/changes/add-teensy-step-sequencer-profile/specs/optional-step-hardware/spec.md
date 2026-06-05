## ADDED Requirements

### Requirement: Step hardware profile opt-in
The profile system SHALL support a separate step hardware profile block that wiring headers may opt into without requiring legacy wiring profiles to declare STEP hardware fields.

#### Scenario: Step hardware profile declared
- **WHEN** a wiring profile declares step hardware support
- **THEN** the profile SHALL provide one STEP switch signal and 8 step-grid LED signals through the step hardware profile block

#### Scenario: Legacy profile omits step hardware block
- **WHEN** a legacy wiring profile does not declare step hardware support
- **THEN** the profile SHALL remain valid when step sequencing is disabled

### Requirement: STEP switch validation
When step sequencing is enabled, the selected profile SHALL declare step hardware support with a usable STEP mode switch signal.

#### Scenario: STEP switch required
- **WHEN** step sequencing is enabled and the selected profile does not declare a usable STEP switch
- **THEN** compilation SHALL fail with a diagnostic that STEP mode requires step hardware support

#### Scenario: STEP switch scanned
- **WHEN** step sequencing is enabled and the selected profile declares a usable STEP switch
- **THEN** the firmware SHALL scan that signal as the selector for STEP mode

### Requirement: Mandatory STEP grid LEDs
When step sequencing is enabled, the selected profile SHALL declare 8 usable step-grid LED signals, one for each visible item on the 8-item page.

#### Scenario: STEP LEDs required
- **WHEN** step sequencing is enabled and the selected profile does not declare 8 usable step-grid LED signals
- **THEN** compilation SHALL fail with a diagnostic that STEP mode requires 8 step-grid LEDs

#### Scenario: STEP LEDs render visible page
- **WHEN** STEP mode is active
- **THEN** each step-grid LED SHALL represent one visible grid item on the current 8-item page

### Requirement: Retrocompatible optional hardware
Step hardware profile support SHALL NOT require existing wiring profiles to add STEP switch or step-grid LED fields when step sequencing is disabled.

#### Scenario: Legacy profile without optional hardware
- **WHEN** an existing Nano or Pro Micro wiring profile is selected
- **THEN** PLAY, CC, REPEAT, and ULTRASONIC mode selection SHALL behave as before

#### Scenario: Legacy profile without sequencer allocation
- **WHEN** step sequencing is disabled at build time
- **THEN** the firmware SHALL not require step hardware support from the selected wiring profile
