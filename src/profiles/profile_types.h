#ifndef PROFILE_TYPES_H
#define PROFILE_TYPES_H

#include <Arduino.h>

static constexpr uint8_t kMappingMaxNotes = 8;

struct BoardProfile {
  const char* name;
  bool hasA6A7;
  bool hasNativeUsbMidi;
  bool supportsSerialMidi;
};

struct WiringProfile {
  const char* name;
  uint8_t encoder1Clk;
  uint8_t encoder1Dt;
  uint8_t encoder1Sw;
  uint8_t encoder2Clk;
  uint8_t encoder2Dt;
  uint8_t encoder2Sw;
  uint8_t muxSig;
  uint8_t muxS0;
  uint8_t muxS1;
  uint8_t muxS2;
  uint8_t muxS3;
  uint8_t lcdClk;
  uint8_t lcdDio;
  uint8_t fader1;
  uint8_t fader2;
  uint8_t led1;
  uint8_t led2;
  uint8_t led3;
  uint8_t led4;
  uint8_t triggerPin;
  uint8_t echoPin;
  uint8_t magnetPin;
  uint8_t swCcChannel;
  uint8_t swRepeatChannel;
  uint8_t swUltrasonicChannel;
  uint8_t swPlayChannel;
  uint8_t pushPins[8];
};

struct MappingProfile {
  const char* name;
  uint8_t midiCcPresets[kMappingMaxNotes];
  uint8_t defaultCcLane1;
  uint8_t defaultCcLane2;
  uint8_t defaultCcValueLane1;
  uint8_t defaultCcValueLane2;
  uint8_t defaultUltrasonicCc;
  uint16_t faderMinValue;
  uint16_t faderMaxValue;
  uint16_t faderThreshold;
  uint8_t minUltrasonicDistanceCm;
  int16_t defaultUltrasonicMaxDistanceCm;
  uint8_t maxUltrasonicDistanceCapCm;
  float ultrasonicSmoothingAlpha;
  uint8_t ultrasonicCcDeadband;
  uint32_t ultrasonicMinUpdateIntervalUs;
};

#endif
