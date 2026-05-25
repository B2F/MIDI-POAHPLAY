#ifndef DISPLAY_IFACE_H
#define DISPLAY_IFACE_H

#include <Arduino.h>
#include "SevenSegmentFun.h"

namespace display_iface {

  struct NonBlockingScrollState {
    bool active = false;
    char text[32] = {0};
    byte textLen = 0;
    byte frameIndex = 0;
    unsigned long nextFrameMs = 0;
    unsigned long stepMs = 150;
  };

  inline SevenSegmentFun*& boundDisplay() {
    static SevenSegmentFun* instance = nullptr;
    return instance;
  }

  inline void init(SevenSegmentFun& display) { boundDisplay() = &display; }

  inline SevenSegmentFun& get() { return *boundDisplay(); }

  inline uint32_t& mutationCounter() {
    static uint32_t counter = 0;
    return counter;
  }

  inline bool& suppressMutationTracking() {
    static bool suppressed = false;
    return suppressed;
  }

  inline void trackMutation() {
    if (!suppressMutationTracking()) {
      mutationCounter()++;
    }
  }

  inline uint32_t getMutationCounter() {
    return mutationCounter();
  }

  inline void begin() { get().begin(); }
  inline void clear() { trackMutation(); get().clear(); }
  inline void setColonOn(bool on) { trackMutation(); get().setColonOn(on); }
  inline void setPrintDelay(uint16_t delayMs) { get().setPrintDelay(delayMs); }
  inline void print(const String& text) { trackMutation(); get().print(text); }
  inline void print(const char* text) { trackMutation(); get().print(text); }
  inline void print(char text) { trackMutation(); get().print(text); }
  inline void scrollingText(const char* text, uint8_t repeats) { trackMutation(); get().scrollingText(text, repeats); }
  inline void blink() { trackMutation(); get().blink(); }
  inline void snake(uint8_t loops, uint16_t delayMs) { trackMutation(); get().snake(loops, delayMs); }

  inline void cancelScroll(NonBlockingScrollState& state) {
    state.active = false;
  }

  inline void startScroll(NonBlockingScrollState& state, const char* label, unsigned long stepMs) {
    byte writePos = 0;
    byte labelLen = strlen(label);
    for (byte i = 0; i < 4 && writePos < sizeof(state.text) - 1; i++) {
      state.text[writePos++] = ' ';
    }
    for (byte i = 0; i < labelLen && writePos < sizeof(state.text) - 1; i++) {
      state.text[writePos++] = label[i];
    }
    for (byte i = 0; i < 4 && writePos < sizeof(state.text) - 1; i++) {
      state.text[writePos++] = ' ';
    }
    state.text[writePos] = '\0';
    state.textLen = writePos;
    state.frameIndex = 0;
    state.nextFrameMs = millis();
    state.stepMs = stepMs;
    state.active = true;
  }

  inline void tickScroll(NonBlockingScrollState& state) {
    if (!state.active) {
      return;
    }
    if ((long)(millis() - state.nextFrameMs) < 0) {
      return;
    }
    if (state.frameIndex + 3 >= state.textLen) {
      state.active = false;
      return;
    }

    char frame[5];
    frame[0] = state.text[state.frameIndex];
    frame[1] = state.text[state.frameIndex + 1];
    frame[2] = state.text[state.frameIndex + 2];
    frame[3] = state.text[state.frameIndex + 3];
    frame[4] = '\0';
    suppressMutationTracking() = true;
    clear();
    setColonOn(false);
    print(frame);
    suppressMutationTracking() = false;

    state.frameIndex++;
    state.nextFrameMs = millis() + state.stepMs;
  }

}  // namespace display_iface

#endif
