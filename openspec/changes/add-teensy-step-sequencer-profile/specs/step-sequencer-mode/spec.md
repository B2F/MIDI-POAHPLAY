## ADDED Requirements

### Requirement: STEP mode selection
The firmware SHALL provide a STEP operating mode when step sequencing is enabled and the selected wiring profile declares a usable STEP switch.

#### Scenario: Select STEP mode
- **WHEN** the STEP switch transitions active while step sequencing is enabled
- **THEN** the firmware SHALL make STEP the active operating mode and display `STEP` on the LCD

#### Scenario: Step sequencing disabled
- **WHEN** step sequencing is disabled at build time
- **THEN** the firmware SHALL compile without requiring a STEP mode switch or step sequencer runtime state

### Requirement: Feature-gated sequencer allocation
The firmware SHALL compile sequencer mode logic, pattern storage, and playback state only when step sequencing is enabled.

#### Scenario: Legacy build excludes sequencer storage
- **WHEN** step sequencing is disabled at build time
- **THEN** the firmware SHALL not allocate sequencer pattern storage or sequencer playback state

### Requirement: Step grid dimensions
The step sequencer SHALL support 8 lanes mapped to the 8 pads and a maximum pattern length of 64 grid items.

#### Scenario: Maximum configured pattern
- **WHEN** the user selects 1:16 grid resolution and 4 bars
- **THEN** the sequencer SHALL address 64 grid items per lane

#### Scenario: Shorter configured pattern
- **WHEN** the user selects fewer bars or a lower grid resolution
- **THEN** playback and editing SHALL wrap at the configured pattern length

### Requirement: Grid resolution control
In STEP mode, the right encoder SHALL select grid resolution among 1:4, 1:8, and 1:16.

#### Scenario: Change grid resolution
- **WHEN** the user turns the right encoder in STEP mode without holding an encoder push
- **THEN** the selected grid resolution SHALL change among 1:4, 1:8, and 1:16 and the display SHALL show the selected resolution

### Requirement: Bar count control
In STEP mode, the left encoder push control SHALL select pattern length from 1 to 4 bars.

#### Scenario: Change bar count
- **WHEN** the user edits bar count with the left encoder push control in STEP mode
- **THEN** the pattern length SHALL change within the range 1 to 4 bars and the display SHALL show the selected bar count

### Requirement: Lane selection
In STEP mode, the left encoder SHALL select the current grid lane corresponding to one of the 8 pads.

#### Scenario: Select lane
- **WHEN** the user turns the left encoder in STEP mode without holding an encoder push
- **THEN** the current lane SHALL change across the 8 pad lanes and wrap at both ends

### Requirement: Eight-item grid page
The step sequencer SHALL present one editable page as 8 grid items.

#### Scenario: Show current page
- **WHEN** STEP mode is active
- **THEN** the visible page SHALL represent 8 consecutive grid items from the current pattern

#### Scenario: Page wraps within pattern
- **WHEN** the current edit position advances beyond the visible 8-item page
- **THEN** the firmware SHALL move to the next 8-item page within the configured pattern length

### Requirement: Recording mode selection
In STEP mode, the right encoder push control SHALL select between live play recording and grid note-selection recording.

#### Scenario: Toggle recording mode
- **WHEN** the user edits recording mode with the right encoder push control in STEP mode
- **THEN** the firmware SHALL switch between live play recording and grid note-selection recording and display the selected recording mode

### Requirement: Grid note selection recording
In grid note-selection recording mode, pad input SHALL toggle notes on the current lane/page grid using the current performance velocity at the time the note is added.

#### Scenario: Add grid note
- **WHEN** the user adds a note to a grid item in grid note-selection recording mode
- **THEN** the sequencer SHALL mark that lane/grid item active and store the current velocity for that recorded note

#### Scenario: Remove grid note
- **WHEN** the user removes an active grid note in grid note-selection recording mode
- **THEN** the sequencer SHALL mark that lane/grid item inactive

### Requirement: Live play recording
In live play recording mode, pad input SHALL be recorded into the current sequencer grid with velocity from the live pad performance.

#### Scenario: Record live pad hit
- **WHEN** live play recording is active and the user plays a pad during sequencer playback
- **THEN** the sequencer SHALL record that pad lane at the quantized current grid item with the played velocity

### Requirement: Sequencer playback
The step sequencer SHALL play active recorded notes according to the selected grid resolution, bar count, lane note mapping, and stored velocity.

#### Scenario: Play recorded note
- **WHEN** playback reaches an active recorded grid item
- **THEN** the firmware SHALL send the lane note using the velocity stored for that recorded note

#### Scenario: Wrap playback
- **WHEN** playback reaches the end of the configured pattern length
- **THEN** playback SHALL continue from the first grid item
