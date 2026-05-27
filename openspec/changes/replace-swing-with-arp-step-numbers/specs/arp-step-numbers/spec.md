## ADDED Requirements

### Requirement: Arp step numbers replace swing control
The system SHALL expose arp step numbers as the authoritative arp-cycle step-count control for arpeggiator playback, and SHALL no longer apply swing values in arp playback.

#### Scenario: Playback uses arp step numbers
- **WHEN** an arp pattern is played
- **THEN** note selection follows the existing arp mode ordering but restarts after the configured step count

#### Scenario: NOTE arp mode remains unchanged
- **WHEN** arp mode is `NOTE`
- **THEN** changing step number does not alter note progression behavior

#### Scenario: Swing is not applied
- **WHEN** arp playback is evaluated
- **THEN** the swing parameter path is ignored or absent

### Requirement: Users can edit arp step number in repeat mode
The system SHALL allow users to set a repeat-mode arp step number through the left encoder push edit path.

#### Scenario: User edits step number
- **WHEN** a user holds left encoder push in repeat mode and rotates the encoder
- **THEN** the arp step number value is updated

#### Scenario: UI shows step number control instead of swing
- **WHEN** a user uses the left encoder push edit in repeat mode
- **THEN** the interface presents arp step number feedback and does not present swing feedback

### Requirement: Arp step numbers are validated
The system SHALL validate arp step number values against defined bounds.

#### Scenario: Default value
- **WHEN** the firmware initializes
- **THEN** arp step number defaults to 8

#### Scenario: Invalid value rejected
- **WHEN** an arp step number outside allowed bounds is submitted
- **THEN** the system clamps the value into the supported range of 2 to 16

### Requirement: Step numbers above pad count add octave rollover
The system SHALL apply octave rollover when arp step number exceeds 8 by continuing through an octave-up pass before cycle restart.

#### Scenario: Step number within one pad cycle
- **WHEN** arp step number is between 2 and 8
- **THEN** playback emits the first N notes of the current arp-mode sequence and then restarts

#### Scenario: Step number above one pad cycle
- **WHEN** arp step number is between 9 and 16
- **THEN** playback emits one full 8-note pass, continues with octave-up notes, and restarts at the configured step count

#### Scenario: UP mode concrete behavior
- **WHEN** scale is MAJ, arp mode is `UP`, and step number is 2
- **THEN** the repeating output sequence is `C, D` before restart

#### Scenario: DOWN mode concrete behavior
- **WHEN** scale is MAJ, arp mode is `DOWN`, and step number is 2
- **THEN** the repeating output sequence is `B, A` before restart
