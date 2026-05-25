#ifndef WIRING_PROFILE_PRO_MICRO_DEFAULT_H
#define WIRING_PROFILE_PRO_MICRO_DEFAULT_H

#include "../profile_types.h"

#define WIRING_PROFILE_TOPOLOGY WIRING_PROFILE_TOPOLOGY_SINGLE_MUX

// Pro Micro default profile (5V/16MHz style clone baseline).
// Notes:
// - Single MUX profile (MUX1 only).
// - Faders are on A2/A3 to avoid A6/A7 (not present on ATmega32U4 boards).
static constexpr SingleMuxWiringProfile kWiringProMicroDefault = {
  {
    "pro_micro_default",
    {
      {PIN(2), PIN(4), MUX_CH1(15)},  // encoder1 [clk, dt, sw]
      {PIN(3), PIN(5), MUX_CH1(14)},  // encoder2 [clk, dt, sw]
    },
    {PIN(12), PIN(11)},  // lcd [clk, dio]
    {PIN(A2), PIN(A3)},  // faders [fader1, fader2]
    {PIN(10), PIN(9), PIN(8), PIN(7)},  // leds [led1..led4]
    {PIN(13), PIN(6)},  // ultrasonic [trigger, echo]
    PIN(A5),  // magnetPin
    MUX_CH1(0),   // swCcChannel
    MUX_CH1(1),   // swRepeatChannel
    MUX_CH1(10),  // swUltrasonicChannel
    MUX_CH1(11),  // swPlayChannel
    {MUX_CH1(4), MUX_CH1(3), MUX_CH1(2), MUX_CH1(5), MUX_CH1(6), MUX_CH1(7), MUX_CH1(8), MUX_CH1(9)},  // pushPins (P1..P8)
  },
  {
    PIN(A0),  // mux1Sig
    PIN(A1),  // mux1S0
    PIN(A2),  // mux1S1
    PIN(A3),  // mux1S2
    PIN(A4),  // mux1S3
  },
};

static constexpr SingleMuxWiringProfile kWiringProfile = kWiringProMicroDefault;

#endif
