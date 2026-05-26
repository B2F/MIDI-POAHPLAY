## Why

Arp mode currently offers note-order motion and rate control, but harmonic movement stays static unless the performer manually changes chord settings mid-play. Adding on-device chord progression sequencing in arp mode enables musically coherent ii-V-I and other progressions without interrupting performance flow.

## What Changes

- Add a progression layer in REPEAT mode that sequences chord qualities over configurable steps while arp timing and slot motion continue to run as-is.
- Add on-device progression editing controls in REPEAT mode (step select, chord assignment per step, progression enable/disable) using existing encoder inputs.
- Preserve backward compatibility: when progression mode is disabled, arp behavior remains identical to current single-chord operation.
- Keep DRUM scale behavior safe by bypassing progression voicing where chord progression semantics do not apply.

## Capabilities

### New Capabilities
- `arp-chord-progression`: Sequenced chord progression playback and editing inside REPEAT mode, including progression on/off and step-based chord mapping.

### Modified Capabilities
- None.

## Impact

- Affected firmware areas: REPEAT mode input handling, arp playback note/chord resolution, display labels for progression edit feedback, reset initialization state.
- No external API changes; behavior remains device-local and MIDI output remains standard Note On/Off.
- Documentation updates required for REPEAT mode control mapping and progression workflow.
