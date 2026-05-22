#ifndef IO_IFACE_H
#define IO_IFACE_H

#include <Arduino.h>

namespace io_iface {

inline void setPinMode(uint8_t pin, uint8_t mode) {
  pinMode(pin, mode);
}

inline void writeDigital(uint8_t pin, uint8_t value) {
  digitalWrite(pin, value);
}

inline int readDigital(uint8_t pin) {
  return digitalRead(pin);
}

inline int readAnalog(uint8_t pin) {
  return analogRead(pin);
}

}  // namespace io_iface

#endif
