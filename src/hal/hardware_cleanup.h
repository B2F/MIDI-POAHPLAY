#ifndef HARDWARE_CLEANUP_H
#define HARDWARE_CLEANUP_H

#include <Arduino.h>

namespace hardware_cleanup {

struct AnalogCleanupConfig {
  uint16_t minValue;
  uint16_t maxValue;
  uint16_t hysteresis;
  uint16_t deadband;
  uint16_t emaAlphaPermille;
};

struct AnalogState {
  uint16_t median[3];
  uint8_t medianCount;
  uint8_t medianIndex;
  int32_t emaValueX1000;
  bool emaInitialized;
  uint16_t lastCommitted;
  bool hasCommitted;
};

struct DigitalDebounceState {
  bool stableValue;
  bool lastRawValue;
  bool initialized;
  unsigned long lastRawChangeUs;
};

AnalogState makeAnalogState();
DigitalDebounceState makeDigitalDebounceState();

uint16_t conditionAnalog(uint16_t rawValue, uint16_t lastOutput, AnalogState& state, const AnalogCleanupConfig& config);
bool debounceDigital(bool rawValue, unsigned long nowUs, unsigned long debounceUs, DigitalDebounceState& state);

}  // namespace hardware_cleanup

#endif
