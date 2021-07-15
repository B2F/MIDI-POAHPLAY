/*
  Input Pull-up Serial
*/

#include "MIDI.h"
#include "TM1637.h"

const int PUSHED = LOW;
const int RELEASED = HIGH;
const int NB_PUSH = 8;
const long BAUD_RATE = 57600;
const int LCD_CLK = 12;
const int LCD_DIO = 11;

struct HairlessMidiSettings : public midi::DefaultSettings
{
   static const bool UseRunningStatus = false;
   static const long BaudRate = BAUD_RATE;
};

MIDI_CREATE_CUSTOM_INSTANCE(HardwareSerial, Serial, MIDI, HairlessMidiSettings);

TM1637 tm(LCD_CLK, LCD_DIO);

int midiChannel = 2;

int pushPin[NB_PUSH] = {3, 4, 5, 6, 7, 8, 9, 10};
int pushNote[NB_PUSH] = {60, 62, 63, 64, 65, 67, 71, 72};
int pushVelocity[NB_PUSH] = {127, 127, 127, 127, 127, 127, 127, 127};
int isPushed[NB_PUSH] = {RELEASED, RELEASED, RELEASED, RELEASED, RELEASED, RELEASED, RELEASED, RELEASED};

void setup() {

  Serial.begin(BAUD_RATE);

  tm.begin();

  tm.display("PLAY");

  for (int i; i < NB_PUSH; i++) {
    pinMode(pushPin[i], INPUT_PULLUP);
  }

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

}

void loop() {

  for (int i = 0; i < NB_PUSH; i++) {

    int sensorVal = digitalRead(pushPin[i]);

    if (sensorVal == PUSHED && isPushed[i] == RELEASED) {
        MIDI.sendNoteOn(pushNote[i], pushVelocity[i], midiChannel);
        isPushed[i] = PUSHED;
        digitalWrite(LED_BUILTIN, HIGH);
    }
    if (sensorVal == RELEASED && isPushed[i] == PUSHED) {
        isPushed[i] = RELEASED;
        MIDI.sendNoteOff(pushNote[i], pushVelocity[i], midiChannel);
        digitalWrite(LED_BUILTIN, LOW);
    }
  }

  delay(10);
}
