#ifndef PROFILE_TYPES_H
#define PROFILE_TYPES_H

#include <Arduino.h>

static constexpr uint8_t kMappingMaxNotes = 8;
static constexpr uint8_t kUnusedSignalId = 0xFF;

#define WIRING_PROFILE_TOPOLOGY_SINGLE_MUX 1
#define WIRING_PROFILE_TOPOLOGY_DUAL_MUX 2

struct BoardProfile {
  const char* name;
  bool hasA6A7;
  bool hasNativeUsbMidi;
  bool supportsSerialMidi;
};

enum SignalSource : uint8_t {
  SIGNAL_SOURCE_PIN = 0,
  SIGNAL_SOURCE_MUX1 = 1,
  SIGNAL_SOURCE_MUX2 = 2,
};

struct SignalRef {
  uint8_t id;
  SignalSource source;
};

constexpr SignalRef PIN(uint8_t pin) {
  return {pin, SIGNAL_SOURCE_PIN};
}

constexpr SignalRef MUX_CH1(uint8_t channel) {
  return {channel, SIGNAL_SOURCE_MUX1};
}

constexpr SignalRef MUX_CH2(uint8_t channel) {
  return {channel, SIGNAL_SOURCE_MUX2};
}

constexpr bool isPinSource(SignalRef signal) {
  return signal.source == SIGNAL_SOURCE_PIN;
}

constexpr bool isMuxSource(SignalRef signal) {
  return signal.source == SIGNAL_SOURCE_MUX1 || signal.source == SIGNAL_SOURCE_MUX2;
}

constexpr bool isMux1Source(SignalRef signal) {
  return signal.source == SIGNAL_SOURCE_MUX1;
}

constexpr bool isMux2Source(SignalRef signal) {
  return signal.source == SIGNAL_SOURCE_MUX2;
}

constexpr bool isA6A7Pin(SignalRef signal) {
#if defined(A6) && defined(A7)
  return isPinSource(signal) && (signal.id == A6 || signal.id == A7);
#else
  return false;
#endif
}

constexpr bool isValidMuxSignal(SignalRef signal) {
  return !isMuxSource(signal) || signal.id < 16;
}

constexpr bool isUsableSignalPin(SignalRef signal) {
  return signal.id != kUnusedSignalId;
}

struct WiringProfile {
  const char* name;
  struct {
    SignalRef clk;
    SignalRef dt;
    SignalRef sw;
  } encoders[2];
  struct {
    SignalRef clk;
    SignalRef dio;
  } lcd;
  SignalRef faders[2];
  SignalRef leds[4];
  struct {
    SignalRef trigger;
    SignalRef echo;
  } ultrasonic;
  SignalRef magnetPin;
  SignalRef swCcChannel;
  SignalRef swRepeatChannel;
  SignalRef swUltrasonicChannel;
  SignalRef swPlayChannel;
  SignalRef pushPins[8];
};

struct MuxPins {
  SignalRef sig;
  SignalRef s0;
  SignalRef s1;
  SignalRef s2;
  SignalRef s3;
};

struct SingleMuxWiringProfile {
  WiringProfile wiring;
  MuxPins mux1;
};

struct DualMuxWiringProfile {
  WiringProfile wiring;
  MuxPins mux1;
  MuxPins mux2;
};

struct ResolvedWiringProfile {
  const char* name;
  struct {
    SignalRef clk;
    SignalRef dt;
    SignalRef sw;
  } encoders[2];
  SignalRef mux1Sig;
  SignalRef mux1S0;
  SignalRef mux1S1;
  SignalRef mux1S2;
  SignalRef mux1S3;
  SignalRef mux2Sig;
  SignalRef mux2S0;
  SignalRef mux2S1;
  SignalRef mux2S2;
  SignalRef mux2S3;
  struct {
    SignalRef clk;
    SignalRef dio;
  } lcd;
  SignalRef faders[2];
  SignalRef leds[4];
  struct {
    SignalRef trigger;
    SignalRef echo;
  } ultrasonic;
  SignalRef magnetPin;
  SignalRef swCcChannel;
  SignalRef swRepeatChannel;
  SignalRef swUltrasonicChannel;
  SignalRef swPlayChannel;
  SignalRef pushPins[8];
};

constexpr SignalRef kUnusedPin = PIN(kUnusedSignalId);

constexpr MuxPins unresolvedMuxPins() {
  return {kUnusedPin, kUnusedPin, kUnusedPin, kUnusedPin, kUnusedPin};
}

constexpr ResolvedWiringProfile resolveWiringProfile(const SingleMuxWiringProfile& profile) {
  return {
    profile.wiring.name,
    {
      {profile.wiring.encoders[0].clk, profile.wiring.encoders[0].dt, profile.wiring.encoders[0].sw},
      {profile.wiring.encoders[1].clk, profile.wiring.encoders[1].dt, profile.wiring.encoders[1].sw}
    },
    profile.mux1.sig,
    profile.mux1.s0,
    profile.mux1.s1,
    profile.mux1.s2,
    profile.mux1.s3,
    kUnusedPin,
    kUnusedPin,
    kUnusedPin,
    kUnusedPin,
    kUnusedPin,
    {profile.wiring.lcd.clk, profile.wiring.lcd.dio},
    {profile.wiring.faders[0], profile.wiring.faders[1]},
    {profile.wiring.leds[0], profile.wiring.leds[1], profile.wiring.leds[2], profile.wiring.leds[3]},
    {profile.wiring.ultrasonic.trigger, profile.wiring.ultrasonic.echo},
    profile.wiring.magnetPin,
    profile.wiring.swCcChannel,
    profile.wiring.swRepeatChannel,
    profile.wiring.swUltrasonicChannel,
    profile.wiring.swPlayChannel,
    {
      profile.wiring.pushPins[0], profile.wiring.pushPins[1], profile.wiring.pushPins[2], profile.wiring.pushPins[3],
      profile.wiring.pushPins[4], profile.wiring.pushPins[5], profile.wiring.pushPins[6], profile.wiring.pushPins[7]
    },
  };
}

constexpr ResolvedWiringProfile resolveWiringProfile(const DualMuxWiringProfile& profile) {
  return {
    profile.wiring.name,
    {
      {profile.wiring.encoders[0].clk, profile.wiring.encoders[0].dt, profile.wiring.encoders[0].sw},
      {profile.wiring.encoders[1].clk, profile.wiring.encoders[1].dt, profile.wiring.encoders[1].sw}
    },
    profile.mux1.sig,
    profile.mux1.s0,
    profile.mux1.s1,
    profile.mux1.s2,
    profile.mux1.s3,
    profile.mux2.sig,
    profile.mux2.s0,
    profile.mux2.s1,
    profile.mux2.s2,
    profile.mux2.s3,
    {profile.wiring.lcd.clk, profile.wiring.lcd.dio},
    {profile.wiring.faders[0], profile.wiring.faders[1]},
    {profile.wiring.leds[0], profile.wiring.leds[1], profile.wiring.leds[2], profile.wiring.leds[3]},
    {profile.wiring.ultrasonic.trigger, profile.wiring.ultrasonic.echo},
    profile.wiring.magnetPin,
    profile.wiring.swCcChannel,
    profile.wiring.swRepeatChannel,
    profile.wiring.swUltrasonicChannel,
    profile.wiring.swPlayChannel,
    {
      profile.wiring.pushPins[0], profile.wiring.pushPins[1], profile.wiring.pushPins[2], profile.wiring.pushPins[3],
      profile.wiring.pushPins[4], profile.wiring.pushPins[5], profile.wiring.pushPins[6], profile.wiring.pushPins[7]
    },
  };
}

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
