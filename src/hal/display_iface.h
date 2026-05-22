#ifndef DISPLAY_IFACE_H
#define DISPLAY_IFACE_H

#include <Arduino.h>
#include "SevenSegmentFun.h"

namespace display_iface {

  inline SevenSegmentFun*& boundDisplay() {
    static SevenSegmentFun* instance = nullptr;
    return instance;
  }

  inline void init(SevenSegmentFun& display) { boundDisplay() = &display; }

  inline SevenSegmentFun& get() { return *boundDisplay(); }

  inline void begin() { get().begin(); }
  inline void clear() { get().clear(); }
  inline void setColonOn(bool on) { get().setColonOn(on); }
  inline void print(const String& text) { get().print(text); }
  inline void print(const char* text) { get().print(text); }
  inline void print(char text) { get().print(text); }
  inline void blink() { get().blink(); }
  inline void snake(uint8_t loops, uint16_t delayMs) { get().snake(loops, delayMs); }

}  // namespace display_iface

#endif
