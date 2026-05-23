#ifndef MIDI_IFACE_H
#define MIDI_IFACE_H

#include <Arduino.h>
#include "MIDI.h"
#include "../config/build_config.h"

#if APP_MIDI_TRANSPORT == MIDI_TRANSPORT_USB
#include <MIDIUSB.h>
#endif

namespace app_midi {

enum RealtimeMessage : uint8_t {
  RT_NONE = 0,
  RT_CLOCK = 0xF8,
  RT_START = 0xFA,
  RT_CONTINUE = 0xFB,
  RT_STOP = 0xFC,
  RT_SONG_POSITION_POINTER = 0xF2,
};

struct HairlessMidiSettings : public midi::DefaultSettings {
  static const bool UseRunningStatus = false;
  static const long BaudRate = 38400;
};

#if APP_MIDI_TRANSPORT == MIDI_TRANSPORT_SERIAL
#if defined(USBCON)
using MidiSerialTransport = Serial_;
#else
using MidiSerialTransport = HardwareSerial;
#endif

MIDI_CREATE_CUSTOM_INSTANCE(MidiSerialTransport, Serial, SerialMidi, HairlessMidiSettings);

inline void begin() {
  SerialMidi.begin();
}

inline bool readRealtime(uint8_t& message) {
  if (!Serial.available()) {
    return false;
  }
  message = static_cast<uint8_t>(Serial.read());
  return true;
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
inline bool& usbFlushPending() {
  static bool pending = false;
  return pending;
}

inline uint8_t toUsbChannel(uint8_t channel) {
  if (channel < 1) {
    return 0;
  }
  if (channel > 16) {
    return 15;
  }
  return static_cast<uint8_t>(channel - 1);
}

inline void sendUsbPacket(uint8_t cin, uint8_t status, uint8_t data1, uint8_t data2) {
  midiEventPacket_t event = {cin, status, data1, data2};
  MidiUSB.sendMIDI(event);
  usbFlushPending() = true;
}

inline void begin() {}

inline bool readRealtime(uint8_t& message) {
  midiEventPacket_t event = MidiUSB.read();
  if (event.header == 0) {
    return false;
  }

  const uint8_t status = event.byte1;
  if (status == RT_CLOCK || status == RT_START || status == RT_CONTINUE ||
      status == RT_STOP || status == RT_SONG_POSITION_POINTER) {
    message = status;
    return true;
  }

  return false;
}

inline void sendControlChange(uint8_t ccNumber, uint8_t ccValue, uint8_t channel) {
  const uint8_t usbChannel = toUsbChannel(channel);
  sendUsbPacket(0x0B, static_cast<uint8_t>(0xB0 | usbChannel), ccNumber, ccValue);
}

inline void sendNoteOn(uint8_t note, uint8_t velocity, uint8_t channel) {
  const uint8_t usbChannel = toUsbChannel(channel);
  sendUsbPacket(0x09, static_cast<uint8_t>(0x90 | usbChannel), note, velocity);
}

inline void sendNoteOff(uint8_t note, uint8_t velocity, uint8_t channel) {
  const uint8_t usbChannel = toUsbChannel(channel);
  sendUsbPacket(0x08, static_cast<uint8_t>(0x80 | usbChannel), note, velocity);
}

inline void flush() {
  if (!usbFlushPending()) {
    return;
  }
  MidiUSB.flush();
  usbFlushPending() = false;
}
#else
#error "Unknown APP_MIDI_TRANSPORT"
#endif

#if APP_MIDI_TRANSPORT == MIDI_TRANSPORT_SERIAL
inline void flush() {}
#endif

}  // namespace app_midi

#endif
