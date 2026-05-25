#include "hardware_cleanup.h"

namespace hardware_cleanup {

namespace {

uint16_t clampToRange(uint16_t value, uint16_t minValue, uint16_t maxValue) {
  if (value <= minValue) {
    return 0;
  }
  if (value >= maxValue) {
    return 1024;
  }
  return value;
}

uint16_t medianOf3(uint16_t a, uint16_t b, uint16_t c) {
  if ((a >= b && a <= c) || (a >= c && a <= b)) {
    return a;
  }
  if ((b >= a && b <= c) || (b >= c && b <= a)) {
    return b;
  }
  return c;
}

}  // namespace

AnalogState makeAnalogState() {
  AnalogState state = {{0, 0, 0}, 0, 0, 0, false, 0, false};
  return state;
}

DigitalDebounceState makeDigitalDebounceState() {
  DigitalDebounceState state = {false, false, false, 0};
  return state;
}

uint16_t conditionAnalog(uint16_t rawValue, uint16_t lastOutput, AnalogState& state, const AnalogCleanupConfig& config) {
  uint16_t clampedRaw = clampToRange(rawValue, config.minValue, config.maxValue);

  state.median[state.medianIndex] = clampedRaw;
  state.medianIndex = (state.medianIndex + 1) % 3;
  if (state.medianCount < 3) {
    state.medianCount++;
  }

  uint16_t filtered = clampedRaw;
  if (state.medianCount == 3) {
    filtered = medianOf3(state.median[0], state.median[1], state.median[2]);
  }

  uint16_t alpha = config.emaAlphaPermille;
  if (alpha > 1000) {
    alpha = 1000;
  }
  if (!state.emaInitialized) {
    state.emaValueX1000 = (int32_t) filtered * 1000;
    state.emaInitialized = true;
  }
  else {
    state.emaValueX1000 = ((state.emaValueX1000 * (1000 - alpha)) + ((int32_t)filtered * 1000 * alpha)) / 1000;
  }

  uint16_t emaValue = (uint16_t) (state.emaValueX1000 / 1000);

  int deltaFromLastOutput = (int) emaValue - (int) lastOutput;
  if (deltaFromLastOutput < 0) {
    deltaFromLastOutput = -deltaFromLastOutput;
  }
  if (deltaFromLastOutput <= config.hysteresis) {
    return lastOutput;
  }

  if (!state.hasCommitted) {
    state.lastCommitted = emaValue;
    state.hasCommitted = true;
    return emaValue;
  }

  int deltaFromCommitted = (int) emaValue - (int) state.lastCommitted;
  if (deltaFromCommitted < 0) {
    deltaFromCommitted = -deltaFromCommitted;
  }
  if (deltaFromCommitted <= config.deadband) {
    return state.lastCommitted;
  }

  state.lastCommitted = emaValue;
  return emaValue;
}

bool debounceDigital(bool rawValue, unsigned long nowUs, unsigned long debounceUs, DigitalDebounceState& state) {
  if (!state.initialized) {
    state.initialized = true;
    state.stableValue = rawValue;
    state.lastRawValue = rawValue;
    state.lastRawChangeUs = nowUs;
    return state.stableValue;
  }

  if (rawValue != state.lastRawValue) {
    state.lastRawValue = rawValue;
    state.lastRawChangeUs = nowUs;
  }

  if ((unsigned long)(nowUs - state.lastRawChangeUs) >= debounceUs) {
    state.stableValue = state.lastRawValue;
  }

  return state.stableValue;
}

}  // namespace hardware_cleanup
