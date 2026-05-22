## ADDED Requirements

### Requirement: Event-driven non-reset mode selection
The firmware MUST select non-reset runtime modes from switch rising-edge events instead of continuous priority arbitration. A rising edge on `SW_CC`, `SW_REPEAT`, `SW_ULTRASONIC`, or `SW_PLAY` SHALL immediately set selected mode to `CC`, `REP`, `ULT`, or `PLAY` respectively.

#### Scenario: CC rising edge selects CC mode
- **WHEN** `SW_CC` transitions from OFF to ON
- **THEN** selected mode becomes `CC`

#### Scenario: Last rising edge wins during overlap
- **WHEN** `SW_CC` is already ON and `SW_REPEAT` transitions from OFF to ON
- **THEN** selected mode becomes `REP`

#### Scenario: Releasing switches does not change selected mode
- **WHEN** a selected mode switch transitions from ON to OFF without a new rising edge on another mode switch
- **THEN** selected mode remains unchanged

### Requirement: Reset override mode and reset action
The firmware MUST enter reset override mode (`RES`) only when all four mode switches (`SW_CC`, `SW_REPEAT`, `SW_ULTRASONIC`, `SW_PLAY`) are ON simultaneously. On entry into `RES`, the firmware MUST execute reset-settings behavior once. On exit from `RES`, the active mode MUST become `PLAY`.

#### Scenario: All four switches ON enters RES and resets once
- **WHEN** all four mode switches become ON at the same time state
- **THEN** active mode becomes `RES` and reset-settings behavior executes once

#### Scenario: RES hold does not retrigger reset
- **WHEN** all four mode switches remain ON across loop iterations
- **THEN** reset-settings behavior is not re-executed

#### Scenario: Releasing any switch exits RES to PLAY
- **WHEN** active mode is `RES` and at least one of the four mode switches turns OFF
- **THEN** active mode becomes `PLAY`

### Requirement: Mode activation display labels
The firmware MUST display a mode label when active mode transitions. The labels SHALL be `REP`, `CC`, `PLAY`, `ULT`, and `RES` for repeat, CC, play/default, ultrasonic, and reset modes.

#### Scenario: Repeat mode transition label
- **WHEN** active mode transitions to repeat mode
- **THEN** display shows `REP`

#### Scenario: Reset mode transition label
- **WHEN** active mode transitions to reset mode
- **THEN** display shows `RES`

### Requirement: Mode-specific control routing
The firmware MUST route fader, non-push encoder, and push-encoder behavior from active mode definitions.

For faders and non-push encoders:
- `PLAY` MUST provide velocity and octave behavior.
- `CC` MUST provide CC value behavior for fader lanes by index order.
- `REP`, `ULT`, and `RES` MUST use non-CC fader/non-push behavior.

For push encoders:
- `PLAY`: left controls scale, right controls chord.
- `REP`: left controls arp, right controls speed.
- `ULT`: left controls ultrasonic CC, right controls ultrasonic distance.
- `CC`: push interactions MUST follow lane-index CC behavior.
- `RES`: left controls MIDI channel, right controls base note.

#### Scenario: CC mode faders always control CC values
- **WHEN** active mode is `CC`
- **THEN** both faders update CC lane values according to lane index order

#### Scenario: Repeat mode push controls
- **WHEN** active mode is `REP` and encoder pushes are used
- **THEN** left push controls arp behavior and right push controls speed behavior

#### Scenario: Reset mode push controls
- **WHEN** active mode is `RES` and encoder pushes are used
- **THEN** left push path controls MIDI channel and right push path controls base note
