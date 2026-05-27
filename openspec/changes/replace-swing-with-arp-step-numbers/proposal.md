## Why

The current swing feature does not match the desired sequencing workflow. Replacing it with arp step numbers gives users direct, deterministic control over arpeggiator timing per step.

## What Changes

- Remove the swing control path from the sequencing/arpeggiator flow.
- Introduce arp step numbers as the new step-level rhythmic control.
- Update relevant UI, state handling, and serialization so projects persist arp step number settings.
- Update validation and playback logic to interpret arp step numbers instead of swing values.

## Capabilities

### New Capabilities
- `arp-step-numbers`: Define, edit, persist, and apply per-step arp numeric values used during playback timing.

### Modified Capabilities
- None.

## Impact

- Affected code: sequencer/arp model, playback engine timing path, editor controls, and project save/load mapping.
- APIs/data contracts: project data shape may replace or deprecate swing-related fields in favor of arp step numbers.
- Dependencies/systems: no new external dependencies expected; internal migration and defaults required for existing data.
