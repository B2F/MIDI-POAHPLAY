## Why

The current AVR targets are too constrained for a useful velocity-aware step sequencer, and the project needs a higher-headroom 3.3 V hardware path for future performance features. A Teensy 4.0 profile provides native USB MIDI, ample memory, and enough GPIO to add a dedicated STEP mode with optional grid hardware while preserving existing Nano and Pro Micro behavior.

## What Changes

- Add an optional STEP operating mode that can be enabled at build time and selected by a dedicated fifth mode switch when the selected wiring profile provides it.
- Add a basic velocity-aware step sequencer with 8-lane pad grid editing, 1:4 to 1:16 grid resolution, and 1 to 4 bar pattern lengths.
- Support two recording modes in STEP mode: live play recording and explicit grid note selection.
- Add optional step-grid LEDs where one page shows 8 grid items and each LED represents one visible grid item.
- Add a Teensy 4.0 board profile and wiring profile path for the expanded 3.3 V hardware revision.
- Require the Teensy hardware path to use a 3.3 V-compatible ultrasonic sensor or documented level shifting for any 5 V echo signal.
- Normalize fader input behavior on Teensy by configuring analog reads to the existing 10-bit firmware range.
- Preserve retrocompatibility: existing Nano and Pro Micro profiles shall compile and run without the STEP switch, step LEDs, Teensy profile, or sequencer feature enabled.

## Capabilities

### New Capabilities

- `step-sequencer-mode`: STEP operating mode behavior, grid resolution, bar length, lane selection, recording modes, velocity capture, and playback semantics.
- `teensy40-hardware-profile`: Teensy 4.0 board/profile support, native USB MIDI assumptions, 3.3 V hardware constraints, and ultrasonic sensor requirements.
- `optional-step-hardware`: Optional fifth STEP mode switch and step-grid LEDs without breaking existing wiring profiles.
- `analog-input-normalization`: Consistent fader behavior across AVR and Teensy ADC configurations by using a normalized 10-bit analog input range.

### Modified Capabilities

- None.

## Impact

- Affects `platformio.ini` by adding a Teensy 4.0 build environment and required platform/library selections.
- Affects `src/config/build_config.h` by adding a build-time step-sequencer feature gate and any required target configuration selectors.
- Affects `src/profiles/*` by adding Teensy 4.0 board/wiring profiles and extending profile schemas for optional STEP switch and step-grid LEDs.
- Affects `powaplay.ino` mode handling, input scanning, display feedback, MIDI playback scheduling, and fader setup when implementation begins.
- May require replacing or documenting the ultrasonic module choice for 3.3 V-compatible hardware.
