#ifndef BUILD_CONFIG_H
#define BUILD_CONFIG_H

// Board selectors
#define BOARD_NANO 1
#define BOARD_PRO_MICRO 2

// Wiring profile header selector.
// Set this to a header under src/profiles/, for example:
//   "wirings/nano_default.h"
//   "wirings/pro_micro_clone_safe.h"
//   "local/local_profiles.h"

// Mapping selectors
#define MAPPING_DEFAULT 1
#define MAPPING_LOCAL 100

// Transport selectors
#define MIDI_TRANSPORT_SERIAL 1
#define MIDI_TRANSPORT_USB 2

// Load local overrides first (if present).
#if __has_include("build_config.local.h")
#include "build_config.local.h"
#endif

// Active profile selectors.
// Override these in src/config/build_config.local.h for local, non-versioned customization.
#ifndef APP_BOARD_PROFILE
#define APP_BOARD_PROFILE BOARD_NANO
#endif

#ifndef APP_WIRING_PROFILE_HEADER
#define APP_WIRING_PROFILE_HEADER "wirings/nano_default.h"
#endif

#ifndef APP_MAPPING_PROFILE
#define APP_MAPPING_PROFILE MAPPING_DEFAULT
#endif

#ifndef APP_MIDI_TRANSPORT
#define APP_MIDI_TRANSPORT MIDI_TRANSPORT_SERIAL
#endif

#endif
