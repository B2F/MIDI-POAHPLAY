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

// Input polarity selectors
#define INPUT_ACTIVE_LOW 0
#define INPUT_ACTIVE_HIGH 1

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

// Mode switches are mux channels SW_CC/SW_REPEAT/SW_ULTRASONIC/SW_PLAY.
#ifndef APP_MODE_SWITCH_ACTIVE_LEVEL
#define APP_MODE_SWITCH_ACTIVE_LEVEL INPUT_ACTIVE_HIGH
#endif

// Encoder pushes are mux channels encoder1Sw/encoder2Sw.
#ifndef APP_ENCODER_PUSH_ACTIVE_LEVEL
#define APP_ENCODER_PUSH_ACTIVE_LEVEL INPUT_ACTIVE_LOW
#endif

// Auto-detect mode switch active state by comparing against boot-time baseline.
// Set to 0 to use APP_MODE_SWITCH_ACTIVE_LEVEL directly.
#ifndef APP_MODE_SWITCH_USE_BASELINE
#define APP_MODE_SWITCH_USE_BASELINE 1
#endif

// Number of active mode switches required to trigger RESET on rising edge.
// Valid range: 2..4 (SW_CC, SW_REPEAT, SW_ULTRASONIC, SW_PLAY).
#ifndef APP_RESET_ACTIVE_SWITCH_COUNT
#define APP_RESET_ACTIVE_SWITCH_COUNT 4
#endif

#if APP_RESET_ACTIVE_SWITCH_COUNT < 2 || APP_RESET_ACTIVE_SWITCH_COUNT > 4
#error "APP_RESET_ACTIVE_SWITCH_COUNT must be between 2 and 4"
#endif

#endif
