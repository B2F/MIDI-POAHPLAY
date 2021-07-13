/*
  Input Pull-up Serial
*/

#include "MIDI.h"
#include "TM1637.h"

const int BAUD_RATE = 57600;

const int MIDI_CHANNEL = 1;

const int LCD_CLK_PIN = 12;
const int LCD_DIO_PIN = 11;

const int PUSH_ON = LOW;
const int PUSH_OFF = HIGH;

struct HairlessMidiSettings : public midi::DefaultSettings
{
   static const bool UseRunningStatus = false;
   static const long BaudRate = BAUD_RATE;
};

MIDI_CREATE_CUSTOM_INSTANCE(HardwareSerial, Serial, MIDI, HairlessMidiSettings);

TM1637 tm(LCD_CLK_PIN, LCD_DIO_PIN);

int pushPin[8] = {3, 4, 5, 6, 7, 8, 9, 10};
int pushVelocity[8] = {127, 127, 127, 127, 127, 127, 127, 127};
int pushNote[8] = {60, 62, 64, 67, 69, 71, 72};
int pushStatus[8] = {HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH};

void setup() {

  Serial.begin(BAUD_RATE);

  MIDI.begin(MIDI_CHANNEL_OMNI);

  Serial.println("here");

  for (int pin : pushPin) {
    pinMode(pushPin[pin], INPUT_PULLUP);
    Serial.println(pushPin[pin]);
  }

  tm.begin();
  tm.clearScreen();

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

}

void loop() {

  for (int pin : pushPin) {

    int status = digitalRead(pin);

    if (status == LOW && status != pushStatus[pin]) {
      pushStatus[pin] = LOW;
      MIDI.sendNoteOn(pushNote[pin], pushVelocity[pin], MIDI_CHANNEL);
      digitalWrite(LED_BUILTIN, HIGH);
    }
    else if (status == HIGH && status != pushStatus[pin]) {
      pushStatus[pin] = HIGH;
      MIDI.sendNoteOff(pushNote[pin], pushVelocity[pin], MIDI_CHANNEL);
      digitalWrite(LED_BUILTIN, LOW);
      Serial.println(status);
    }
  }

  delay(100);
}
