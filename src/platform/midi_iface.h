#ifndef MIDI_IFACE_H
#define MIDI_IFACE_H

#include <Arduino.h>
#include "MIDI.h"
#include "../config/build_config.h"

namespace app_midi {

struct HairlessMidiSettings : public midi::DefaultSettings {
  static const bool UseRunningStatus = false;
  static const long BaudRate = 38400;
};

#if APP_MIDI_TRANSPORT == MIDI_TRANSPORT_SERIAL
MIDI_CREATE_CUSTOM_INSTANCE(HardwareSerial, Serial, SerialMidi, HairlessMidiSettings);

inline void begin() {
  SerialMidi.begin();
}

inline void sendControlChange(uint8_t ccNumber, uint8_t ccValue, uint8_t channel) {
  SerialMidi.sendControlChange(ccNumber, ccValue, channel);
}

inline void sendNoteOn(uint8_t note, uint8_t velocity, uint8_t channel) {
  SerialMidi.sendNoteOn(note, velocity, channel);
}

inline void sendNoteOff(uint8_t note, uint8_t velocity, uint8_t channel) {
  SerialMidi.sendNoteOff(note, velocity, channel);
}
#elif APP_MIDI_TRANSPORT == MIDI_TRANSPORT_USB
inline void begin() {}
inline void sendControlChange(uint8_t, uint8_t, uint8_t) {}
inline void sendNoteOn(uint8_t, uint8_t, uint8_t) {}
inline void sendNoteOff(uint8_t, uint8_t, uint8_t) {}
#else
#error "Unknown APP_MIDI_TRANSPORT"
#endif

}  // namespace app_midi

#endif
