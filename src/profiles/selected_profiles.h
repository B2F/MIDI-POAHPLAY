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

static constexpr WiringProfile kSelectedWiringProfile = kWiringProfile;

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
              (kSelectedWiringProfile.fader1 != A6 && kSelectedWiringProfile.fader1 != A7 &&
               kSelectedWiringProfile.fader2 != A6 && kSelectedWiringProfile.fader2 != A7),
              "Selected board profile does not support A6/A7 fader wiring");

#endif
