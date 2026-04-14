# MIDI P0Ah PLAy (proto-serial)

DIY performance controller firmware (Arduino sketch) that outputs **MIDI over Serial** and can also **follow incoming MIDI clock** to sync a pad note-repeat engine.

Everything is in [`powaplay.ino`](./powaplay.ino).

## What You Get

**Inputs**

- 8 pads (momentary buttons) through a CD74HC4067 multiplexer
- 2 rotary encoders (+ 2 encoder push switches, also read on the mux)
- 2 faders
- 4 mode switches (mux channels `0`, `1`, `10`, `11`)
- Optional ultrasonic sensor (HC-SR04)

**Outputs**

- MIDI out (Serial bridge):
  - Note On/Off (with optional chords and scale remap)
  - Control Change (CC)
- Tempo LEDs (4 outputs) driven from incoming MIDI clock
- Built-in LED + `MAGNET` output while a note is on
- TM1637 4-digit display for feedback (velocity, octave, CC, repeat speed, etc.)

## Quick Start

1. Open [`powaplay.ino`](./powaplay.ino) in the Arduino IDE.
2. Install dependencies (Arduino Library Manager, or your preferred method):
   - `MIDI` (FortySevenEffects)
   - `Encoder` (Paul Stoffregen)
   - A TM1637 7-segment library that provides `SevenSegmentTM1637.h` / `SevenSegmentFun.h`
   - `light_CD74HC4067` (CD74HC4067 multiplexer)
   - `HCSR04`
3. Select your board and port, then upload.
4. On the computer, bridge the serial port to MIDI:
   - Hairless MIDI <-> Serial Bridge is the intended workflow (custom MIDI settings in the sketch).
5. In your DAW:
   - Select the created MIDI input as the controller input.
   - If you want tempo-synced repeats, send MIDI clock/start/stop to the controller (through the same bridge).

## MIDI I/O

**Serial MIDI**

- Baud rate: `38400` (`BAUD_RATE`)
- MIDI channel: `midiChannel` (default `2`)

**MIDI output**

- Notes: `sendNote()` sends NoteOn/NoteOff (and expands to a chord if enabled).
- CC: `MIDI.sendControlChange(...)` from CC mode and from the ultrasonic sensor.

**MIDI input (from Serial)**

The sketch listens for:

- `MIDI_START` / `MIDI_STOP` / `MIDI_CONTINUE`
- `MIDI_CLOCK` (used to compute BPM and drive the tempo LEDs)
- `MIDI_SONG_POSITION_POINTER` (display only; TODO)

## Controls (Practical)

### Pads

- Pressing a pad triggers `playPush(pad, on/off)`.
- Default pad base notes are initialized from `globalStartNote` (default `48`) in `setup()` as `pushNote[pad] = globalStartNote + pad`.
- Depending on the selected scale, some pads can be disabled (`UNASSIGNED`) and will produce no note.

Tip: a lot of “edit” actions apply to the *selected pad* (`selectedPushPin`). Press a pad first before trying to edit per-pad settings.

### Performance Mode (normal play)

When `CC` mode is OFF:

- Fader 1 (`F1`) edits velocity (global or per-pad if the pad is locked).
- Fader 2 (`F2`) edits octave (`-4` to `+6`) and sets `globalNoteOffset = octave * 12`.
- Turning an encoder (when its push switch is not held) fine-adjusts velocity or octave.

**Chords and scales**

- Hold the encoder push switch that triggers *scale selection* and turn the corresponding encoder to cycle scales:
  - `SEMI` (no remap)
  - `MAJ`, `MIN_`, `BLUE`, `BLU_`
- Hold the encoder push switch that triggers *chord selection* and turn the corresponding encoder to cycle chords:
  - `NOTE` (no chord)
  - `MAJ`, `MIN`, `AUG`

### CC Mode

Enable CC mode via the mux switch `SW_CC` (mux channel `0`).

- Fader 1 and fader 2 send CC values for two CC lanes (`midiCC[0]` and `midiCC[1]`).
- Turning an encoder adjusts the CC value (unless you are holding the corresponding encoder push switch).
- Holding an encoder push switch switches that lane into “CC select”:
  - Turn the encoder to change the CC number (`midiCC[x]`).
  - Hold one or more pads to pick a CC preset from `midiCCPresets[]` (pads map to presets).

### Note Repeat Mode

Enable repeat mode via the mux switch `SW_REPEAT` (mux channel `1`).

- Pads you hold will repeat in sync with the internal tempo (which follows incoming MIDI clock when present).
- Hold the repeat-speed modifier and turn the encoder to change repeat speed divisor (`1..64`). The display shows `1N` with the colon lit.

**Repeat lock**

- While in repeat mode, you can lock repeating on the currently held pads:
  - Hold the lock modifier: locks the held pads (`repeatIsLocked[p] = true`)
  - Hold the unlock modifier: unlocks the held pads (`repeatIsLocked[p] = false`)

### Ultrasonic Mode (HC-SR04)

Enable ultrasonic mode via the mux switch `SW_ULTRASONIC` (mux channel `10`).

- The sensor distance is mapped to a CC value and sent continuously.
- Holding the appropriate encoder push switch lets you edit:
  - `ultrasonicCC` (CC number)
  - `maxUltrasonicDistanceCm` (range)

### Init / Reset Mode

Enable init mode via the mux switch `SW_PLAY` (mux channel `11`, used as `INIT_MASK` in this branch).

- In init mode, encoders adjust:
  - MIDI channel
  - base note offset (global/per-pad depending on pad selection/lock)
- There is also a “reinit” gesture: when leaving init mode, holding the left encoder push switch triggers `reinit()` (display shows `init`).

## Hardware / Wiring

Multiplexer:

- CD74HC4067 signal -> `A0` (`MUXSIG`) with `INPUT_PULLUP`
- Address pins -> `A1, A2, A3, A4`
- Inputs are treated as active-low for pads (`PUSHED == LOW`).
- Mode switches are read as `true` when the mux input reads HIGH in `readSwitches()`.

Encoders:

- Encoder A/B pins:
  - P1: `P1CLK=2`, `P1DT=4`
  - P2: `P2CLK=3`, `P2DT=5`
- Encoder push switches are read through the mux:
  - `P1SW` is mux channel `15`
  - `P2SW` is mux channel `14`

Faders:

- `F1 = A6`, `F2 = A7`

Display (TM1637-style):

- `LCD_CLK = 12`, `LCD_DIO = 11`

LED outputs:

- `L1 = 10`, `L2 = 9`, `L3 = 8`, `L4 = 7`

Ultrasonic:

- `triggerPin = 13`, `echoPin = 6`

Magnet output:

- `MAGNET = A5` (set HIGH while notes are on)

Board notes:

- `A6/A7` are not present on some boards (for example Uno). Use a board that has them (Nano/Pro Mini) or change `F1/F2`.
- MIDI is on `Serial` (hardware UART). On native-USB boards you may need a different serial interface and to adjust `MIDI_CREATE_CUSTOM_INSTANCE(...)`.

## Common Tweaks

In [`powaplay.ino`](./powaplay.ino):

- `midiChannel` (default `2`)
- `globalStartNote` (default `48`)
- `midiCCPresets[]` (which CC numbers the pads select in CC mode)
- Fader threshold/feel: `MAX_FADER_VALUE`, `MIN_FADER_VALUE`, `FADER_THRESHOLD`

## Troubleshooting

- No MIDI:
  - Confirm your bridge baud rate is **38400**.
  - Confirm the DAW is using the correct virtual MIDI port.
- Repeats not synced:
  - This sketch expects incoming MIDI clock on the serial MIDI input path. Make sure your bridge routes clock from the DAW to the Arduino.
- Some pads produce no sound:
  - In some scales, pads can be `UNASSIGNED` and are intentionally muted.

## License

No `LICENSE` file is currently present in the repository.
