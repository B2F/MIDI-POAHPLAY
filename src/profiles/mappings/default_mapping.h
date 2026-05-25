#ifndef DEFAULT_MAPPING_PROFILE_H
#define DEFAULT_MAPPING_PROFILE_H

#include "../profile_types.h"

static constexpr MappingProfile kDefaultMapping = {
  "default",
  {
    91,   // Pad 1 preset CC
    92,   // Pad 2 preset CC
    93,   // Pad 3 preset CC
    94,   // Pad 4 preset CC
    95,   // Pad 5 preset CC
    98,   // Pad 6 preset CC
    99,   // Pad 7 preset CC
    100,  // Pad 8 preset CC
  },
  20,   // defaultCcLane1
  21,   // defaultCcLane2
  63,   // defaultCcValueLane1
  63,   // defaultCcValueLane2
  100,  // defaultUltrasonicCc
  50,   // faderMinValue
  970,  // faderMaxValue
  30,   // faderThreshold
  2,    // faderDeadband
  450,  // faderEmaAlphaPermille
  12000, // faderCcMinUpdateIntervalUs
  2,    // minUltrasonicDistanceCm
  10,   // defaultUltrasonicMaxDistanceCm
  35,   // maxUltrasonicDistanceCapCm
  0.35f, // ultrasonicSmoothingAlpha
  2,    // ultrasonicCcDeadband
  15000, // ultrasonicMinUpdateIntervalUs
};

#endif
