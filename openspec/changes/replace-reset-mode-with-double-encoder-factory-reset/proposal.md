## Why

The current reset mode adds extra UI flow and user confusion for a task that should be immediate and predictable. Replacing reset mode with a direct double-encoder push gesture simplifies recovery actions and aligns with fast hardware-first interaction.

## What Changes

- Remove reset mode behavior and any user flow that enters a dedicated reset state.
- Add a double-encoder push gesture that triggers factory settings reset directly.
- Update reset messaging and interaction feedback to describe the new gesture and outcome.
- Ensure factory reset invocation remains intentional (gesture-based) and is not accidentally triggered by normal single presses.

## Capabilities

### New Capabilities
- `factory-reset-gesture`: Defines the double-encoder push gesture, detection window, and reset trigger behavior.

### Modified Capabilities
- None.

## Impact

- Affected code: input handling and state management in firmware (`powaplay.ino`, `src/`).
- Affected behavior: user reset interaction model and associated user-facing instructions/documentation.
- No external API or dependency changes are expected.
