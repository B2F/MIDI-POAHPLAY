## Why

Mode changes currently trigger a scrolling label animation that can block loop responsiveness at the exact moment performers are switching modes. During live play this feels like controls are frozen, causing missed gestures and timing errors.

## What Changes

- Replace blocking mode-change scrolling with immediate, non-blocking mode label updates.
- Ensure control processing (pads, encoders, faders, repeat/arp timing) continues without pause during mode transitions.
- Add interaction-priority behavior so active input can suppress mode label rendering for that cycle.
- Preserve reset safety behavior (panic + reinit) while changing only transition-display behavior.

## Capabilities

### New Capabilities
- `nonblocking-mode-transition-feedback`: Mode transitions provide visual feedback without interrupting input scanning or control updates.

### Modified Capabilities
- None.

## Impact

- Affected code: `powaplay.ino` mode-label transition path (`showModeIfChanged`, loop integration), and possibly display wrapper usage.
- No external API or dependency changes.
- Behavioral impact is user-facing runtime responsiveness during mode switching.
