#ifndef SELECTED_PROFILES_H
#define SELECTED_PROFILES_H

#include "../config/build_config.h"
#include "profile_types.h"
#include "boards/nano.h"
#include "boards/pro_micro.h"
#include "mappings/default_mapping.h"

#include APP_WIRING_PROFILE_HEADER

#if APP_MAPPING_PROFILE == MAPPING_LOCAL
#if __has_include("local/local_profiles.h")
// Local mapping file may also define a wiring alias (kWiringProfile).
// Rename it during this include so local mapping can be consumed without
// clashing with the active wiring profile header selection.
#define kWiringProfile kIgnoredLocalWiringProfile
#include "local/local_profiles.h"
#undef kWiringProfile
#define HAS_LOCAL_PROFILES 1
#else
#define HAS_LOCAL_PROFILES 0
#endif
#else
#define HAS_LOCAL_PROFILES 0
#endif

#if APP_BOARD_PROFILE == BOARD_NANO
static constexpr BoardProfile kSelectedBoardProfile = kBoardNano;
#elif APP_BOARD_PROFILE == BOARD_PRO_MICRO
static constexpr BoardProfile kSelectedBoardProfile = kBoardProMicro;
#else
#error "Unknown APP_BOARD_PROFILE"
#endif

#if !defined(WIRING_PROFILE_TOPOLOGY)
#error "Wiring profile header must define WIRING_PROFILE_TOPOLOGY"
#endif

#if WIRING_PROFILE_TOPOLOGY == WIRING_PROFILE_TOPOLOGY_SINGLE_MUX
static constexpr ResolvedWiringProfile kSelectedWiringProfile = resolveWiringProfile(kWiringProfile);
#elif WIRING_PROFILE_TOPOLOGY == WIRING_PROFILE_TOPOLOGY_DUAL_MUX
static constexpr ResolvedWiringProfile kSelectedWiringProfile = resolveWiringProfile(kWiringProfile);
#else
#error "Unknown WIRING_PROFILE_TOPOLOGY"
#endif

#if APP_MAPPING_PROFILE == MAPPING_DEFAULT
static constexpr MappingProfile kSelectedMappingProfile = kDefaultMapping;
#elif APP_MAPPING_PROFILE == MAPPING_LOCAL
#if HAS_LOCAL_PROFILES
static constexpr MappingProfile kSelectedMappingProfile = kLocalMappingProfile;
#else
#error "APP_MAPPING_PROFILE=MAPPING_LOCAL but src/profiles/local/local_profiles.h is missing"
#endif
#else
#error "Unknown APP_MAPPING_PROFILE"
#endif

static_assert(kSelectedBoardProfile.supportsSerialMidi || APP_MIDI_TRANSPORT != MIDI_TRANSPORT_SERIAL,
              "Selected board profile does not support serial MIDI transport");

static_assert(kSelectedBoardProfile.hasNativeUsbMidi || APP_MIDI_TRANSPORT != MIDI_TRANSPORT_USB,
              "Selected board profile does not support USB MIDI transport");

static_assert(kSelectedBoardProfile.hasA6A7 ||
              (!isA6A7Pin(kSelectedWiringProfile.faders[0]) && !isA6A7Pin(kSelectedWiringProfile.faders[1])),
              "Selected board profile does not support A6/A7 fader wiring");

static_assert(isPinSource(kSelectedWiringProfile.mux1Sig) &&
              isPinSource(kSelectedWiringProfile.encoders[0].clk) &&
              isPinSource(kSelectedWiringProfile.encoders[0].dt) &&
              isPinSource(kSelectedWiringProfile.encoders[1].clk) &&
              isPinSource(kSelectedWiringProfile.encoders[1].dt) &&
              isPinSource(kSelectedWiringProfile.mux1S0) &&
              isPinSource(kSelectedWiringProfile.mux1S1) &&
              isPinSource(kSelectedWiringProfile.mux1S2) &&
              isPinSource(kSelectedWiringProfile.mux1S3) &&
              isPinSource(kSelectedWiringProfile.mux2Sig) &&
              isPinSource(kSelectedWiringProfile.mux2S0) &&
              isPinSource(kSelectedWiringProfile.mux2S1) &&
              isPinSource(kSelectedWiringProfile.mux2S2) &&
              isPinSource(kSelectedWiringProfile.mux2S3) &&
              isPinSource(kSelectedWiringProfile.lcd.clk) &&
              isPinSource(kSelectedWiringProfile.lcd.dio) &&
              isPinSource(kSelectedWiringProfile.leds[0]) &&
              isPinSource(kSelectedWiringProfile.leds[1]) &&
              isPinSource(kSelectedWiringProfile.leds[2]) &&
              isPinSource(kSelectedWiringProfile.leds[3]) &&
              isPinSource(kSelectedWiringProfile.ultrasonic.trigger) &&
              isPinSource(kSelectedWiringProfile.ultrasonic.echo) &&
              isPinSource(kSelectedWiringProfile.magnetPin),
              "Selected wiring uses mux source for a signal that currently requires native pins");

static_assert(isValidMuxSignal(kSelectedWiringProfile.encoders[0].sw) &&
              isValidMuxSignal(kSelectedWiringProfile.encoders[1].sw) &&
              isValidMuxSignal(kSelectedWiringProfile.faders[0]) &&
              isValidMuxSignal(kSelectedWiringProfile.faders[1]) &&
              isValidMuxSignal(kSelectedWiringProfile.swCcChannel) &&
              isValidMuxSignal(kSelectedWiringProfile.swRepeatChannel) &&
              isValidMuxSignal(kSelectedWiringProfile.swUltrasonicChannel) &&
              isValidMuxSignal(kSelectedWiringProfile.swPlayChannel) &&
              isValidMuxSignal(kSelectedWiringProfile.pushPins[0]) &&
              isValidMuxSignal(kSelectedWiringProfile.pushPins[1]) &&
              isValidMuxSignal(kSelectedWiringProfile.pushPins[2]) &&
              isValidMuxSignal(kSelectedWiringProfile.pushPins[3]) &&
              isValidMuxSignal(kSelectedWiringProfile.pushPins[4]) &&
              isValidMuxSignal(kSelectedWiringProfile.pushPins[5]) &&
              isValidMuxSignal(kSelectedWiringProfile.pushPins[6]) &&
              isValidMuxSignal(kSelectedWiringProfile.pushPins[7]),
              "Mux channel wiring must be in the range [0, 15]");

#if WIRING_PROFILE_TOPOLOGY == WIRING_PROFILE_TOPOLOGY_SINGLE_MUX
static_assert(!isMux2Source(kSelectedWiringProfile.encoders[0].sw) &&
              !isMux2Source(kSelectedWiringProfile.encoders[1].sw) &&
              !isMux2Source(kSelectedWiringProfile.faders[0]) &&
              !isMux2Source(kSelectedWiringProfile.faders[1]) &&
              !isMux2Source(kSelectedWiringProfile.swCcChannel) &&
              !isMux2Source(kSelectedWiringProfile.swRepeatChannel) &&
              !isMux2Source(kSelectedWiringProfile.swUltrasonicChannel) &&
              !isMux2Source(kSelectedWiringProfile.swPlayChannel) &&
              !isMux2Source(kSelectedWiringProfile.pushPins[0]) &&
              !isMux2Source(kSelectedWiringProfile.pushPins[1]) &&
              !isMux2Source(kSelectedWiringProfile.pushPins[2]) &&
              !isMux2Source(kSelectedWiringProfile.pushPins[3]) &&
              !isMux2Source(kSelectedWiringProfile.pushPins[4]) &&
              !isMux2Source(kSelectedWiringProfile.pushPins[5]) &&
              !isMux2Source(kSelectedWiringProfile.pushPins[6]) &&
              !isMux2Source(kSelectedWiringProfile.pushPins[7]),
              "Single-mux wiring cannot route inputs through MUX2"
);
#endif

#endif
