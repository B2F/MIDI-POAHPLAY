#ifndef WIRING_PROFILE_NANO_DEFAULT_H
#define WIRING_PROFILE_NANO_DEFAULT_H

#include "../profile_types.h"

static constexpr WiringProfile kWiringNanoDefault = {
  "nano_default",
  2,   // encoder1Clk
  4,   // encoder1Dt
  15,  // encoder1Sw (mux channel)
  3,   // encoder2Clk
  5,   // encoder2Dt
  14,  // encoder2Sw (mux channel)
  A0,  // muxSig
  A1,  // muxS0
  A2,  // muxS1
  A3,  // muxS2
  A4,  // muxS3
  12,  // lcdClk
  11,  // lcdDio
  A6,  // fader1
  A7,  // fader2
  10,  // led1
  9,   // led2
  8,   // led3
  7,   // led4
  13,  // triggerPin
  6,   // echoPin
  A5,  // magnetPin
  0,   // swCcChannel
  1,   // swRepeatChannel
  10,  // swUltrasonicChannel
  11,  // swPlayChannel
  {4, 3, 2, 5, 6, 7, 8, 9},  // pushPins (P1..P8)
};

static constexpr WiringProfile kWiringProfile = kWiringNanoDefault;

#endif
