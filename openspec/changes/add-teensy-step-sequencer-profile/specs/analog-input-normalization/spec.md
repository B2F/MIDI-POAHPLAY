## ADDED Requirements

### Requirement: Ten-bit fader read range
The firmware SHALL configure analog reads so fader logic receives values in the existing 10-bit range used by the current mapping logic.

#### Scenario: Configure Teensy analog resolution
- **WHEN** the Teensy 4.0 target initializes hardware
- **THEN** the firmware SHALL call `analogReadResolution(10)` before fader values are read

### Requirement: Preserve existing fader mapping behavior
Analog input normalization SHALL preserve existing fader threshold, deadband, smoothing, and MIDI value mapping behavior for legacy profiles.

#### Scenario: Build AVR profile
- **WHEN** an AVR profile is built
- **THEN** fader behavior SHALL remain compatible with the existing 0 to 1023 analog input assumptions

#### Scenario: Build Teensy profile
- **WHEN** the Teensy profile is built
- **THEN** fader behavior SHALL use the same effective 0 to 1023 input range as the AVR profiles
