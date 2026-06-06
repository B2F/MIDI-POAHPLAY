## ADDED Requirements

### Requirement: Teensy 4.0 board profile
The profile system SHALL provide a Teensy 4.0 board profile for the expanded sequencer hardware target.

#### Scenario: Select Teensy 4.0 profile
- **WHEN** the build selects the Teensy 4.0 board profile
- **THEN** the firmware SHALL compile with Teensy 4.0 board capabilities and native USB MIDI transport support

### Requirement: Teensy 4.0 build environment
The project SHALL provide a PlatformIO build environment for the Teensy 4.0 sequencer target.

#### Scenario: Build Teensy target
- **WHEN** the Teensy 4.0 environment is built
- **THEN** PlatformIO SHALL use the Teensy platform, selected Teensy wiring profile, USB MIDI transport, and step-sequencer feature flag

### Requirement: 3.3 V hardware assumptions
The Teensy 4.0 hardware profile SHALL treat GPIO and analog inputs as 3.3 V logic and SHALL NOT require direct 5 V input tolerance.

#### Scenario: Document 3.3 V logic domain
- **WHEN** the Teensy 4.0 profile is documented
- **THEN** the documentation SHALL state that control inputs, mux logic, faders, encoder signals, and switch signals must operate in the 3.3 V logic domain

### Requirement: Ultrasonic sensor compatibility
The Teensy 4.0 sequencer hardware path SHALL use a 3.3 V-compatible ultrasonic sensor or require level shifting for any 5 V echo output.

#### Scenario: 3.3 V ultrasonic module
- **WHEN** the hardware uses a 3.3 V-compatible ultrasonic module
- **THEN** the sensor echo signal MAY connect directly to a Teensy GPIO input

#### Scenario: 5 V ultrasonic echo
- **WHEN** the hardware uses an ultrasonic module whose echo output is 5 V
- **THEN** the wiring documentation SHALL require level shifting before the signal reaches a Teensy GPIO input

### Requirement: Existing board profiles remain valid
Adding the Teensy 4.0 profile SHALL NOT invalidate existing Nano or Pro Micro profile selection when the step sequencer is disabled.

#### Scenario: Build legacy profiles
- **WHEN** Nano or Pro Micro profiles are built with step sequencing disabled
- **THEN** the builds SHALL NOT require Teensy-specific configuration, a STEP switch, or step-grid LEDs
