## 1. Build Configuration and Profiles

- [ ] 1.1 Add a disabled-by-default `APP_ENABLE_STEP_SEQUENCER` build flag in the centralized build configuration.
- [ ] 1.2 Add a Teensy 4.0 board selector and board profile with native USB MIDI and 3.3 V logic assumptions.
- [ ] 1.3 Add a PlatformIO Teensy 4.0 environment that selects the Teensy board profile, Teensy wiring profile, USB MIDI transport, and step-sequencer feature flag.
- [ ] 1.4 Add documentation for Teensy 4.0 3.3 V wiring expectations and supported ultrasonic sensor options or required echo level shifting.

## 2. Optional Hardware Profile Schema

- [ ] 2.1 Add a separate `StepHardwareProfile` schema with one STEP mode switch signal and 8 step-grid LED signals.
- [ ] 2.2 Add an explicit wiring-header opt-in marker for step hardware support.
- [ ] 2.3 Keep legacy single-mux and dual-mux wiring profiles valid without declaring step hardware support.
- [ ] 2.4 Add compile-time validation that requires explicit step hardware support when `APP_ENABLE_STEP_SEQUENCER` is enabled.
- [ ] 2.5 Add compile-time validation that requires a usable STEP switch when `APP_ENABLE_STEP_SEQUENCER` is enabled.
- [ ] 2.6 Add compile-time validation that requires 8 usable step-grid LED signals when `APP_ENABLE_STEP_SEQUENCER` is enabled.
- [ ] 2.7 Create a Teensy 4.0 sequencer wiring profile with step hardware support, existing controls, faders, muxes, display, and ultrasonic signals.

## 3. Analog Input Normalization

- [ ] 3.1 Configure Teensy hardware initialization to call `analogReadResolution(10)` before fader reads.
- [ ] 3.2 Verify AVR targets keep existing 0 to 1023 fader assumptions unchanged.
- [ ] 3.3 Verify Teensy fader reads feed the existing threshold, deadband, smoothing, and MIDI value mapping paths correctly.

## 4. STEP Mode Integration

- [ ] 4.1 Add STEP to runtime mode selection only when step sequencing is enabled.
- [ ] 4.2 Scan the optional STEP switch and select STEP mode on active edge.
- [ ] 4.3 Display `STEP` when STEP mode is selected.
- [ ] 4.4 Preserve PLAY, CC, REPEAT, and ULTRASONIC mode behavior when step sequencing is disabled.

## 5. Sequencer Data Model and Controls

- [ ] 5.1 Add a sequencer pattern model for 8 pad lanes and up to 64 grid items.
- [ ] 5.2 Store velocity per recorded note/grid item.
- [ ] 5.3 Implement grid resolution selection with the right encoder for 1:4, 1:8, and 1:16.
- [ ] 5.4 Implement bar-count selection with the left encoder push control for 1 to 4 bars.
- [ ] 5.5 Implement lane selection with the left encoder across the 8 pad lanes.
- [ ] 5.6 Implement recording mode selection with the right encoder push control for live play recording and grid note-selection recording.
- [ ] 5.7 Implement 8-item page tracking and wrap behavior within the configured pattern length.

## 6. Recording and Playback Behavior

- [ ] 6.1 Implement grid note-selection recording that toggles notes on the selected lane/page.
- [ ] 6.2 Capture current performance velocity when a note is added in grid note-selection mode.
- [ ] 6.3 Implement live play recording that records pad hits into the quantized current grid item.
- [ ] 6.4 Capture live pad velocity for live-recorded notes.
- [ ] 6.5 Implement sequencer playback that sends lane notes using stored per-note velocity.
- [ ] 6.6 Implement sequencer pattern wrap at the configured pattern length.
- [ ] 6.7 Ensure sequencer note-off/gate handling avoids stuck notes during playback, mode changes, and pattern edits.

## 7. Step-Grid LED Feedback

- [ ] 7.1 Initialize the 8 required step-grid LEDs when step sequencing is enabled.
- [ ] 7.2 Render each visible 8-item page item to the corresponding step-grid LED.
- [ ] 7.3 Update step-grid LEDs as the current page, lane, or pattern contents change.
- [ ] 7.4 Keep profiles without step hardware support valid when step sequencing is disabled.

## 8. Verification

- [ ] 8.1 Build Nano with step sequencing disabled and confirm no STEP hardware is required.
- [ ] 8.2 Build Pro Micro with step sequencing disabled and confirm no STEP hardware is required.
- [ ] 8.3 Build Teensy 4.0 sequencer target successfully.
- [ ] 8.4 Verify STEP switch selects STEP mode and displays `STEP` on Teensy hardware.
- [ ] 8.5 Verify right encoder cycles 1:4, 1:8, and 1:16 grid resolutions.
- [ ] 8.6 Verify left encoder push selects 1 to 4 bars.
- [ ] 8.7 Verify left encoder selects all 8 lanes and wraps at both ends.
- [ ] 8.8 Verify right encoder push selects live play recording and grid note-selection recording.
- [ ] 8.9 Verify grid note-selection mode stores current velocity when adding notes.
- [ ] 8.10 Verify live play recording stores played velocity at the quantized grid item.
- [ ] 8.11 Verify 8 step-grid LEDs represent the current 8-item page.
- [ ] 8.12 Verify fader behavior remains stable on Teensy after `analogReadResolution(10)`.
- [ ] 8.13 Verify ultrasonic input wiring is 3.3 V-safe on the Teensy hardware path.
