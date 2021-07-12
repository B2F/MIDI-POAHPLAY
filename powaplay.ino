/*
  Input Pull-up Serial
*/

#include "MIDI.h"
#include "TM1637.h"

// struct MySettings : public midi::DefaultSettings {
//   static const long BaudRate = 57600;
// };

struct HairlessMidiSettings : public midi::DefaultSettings
{
   static const bool UseRunningStatus = false;
   static const long BaudRate = 57600;
};

MIDI_CREATE_CUSTOM_INSTANCE(HardwareSerial, Serial, MIDI, HairlessMidiSettings);
// MIDI_CREATE_CUSTOM_INSTANCE();

TM1637 tm(12, 11);

bool sensor1IsActive = false;

void setup() {
  //start serial connection
  Serial.begin(57600);

  tm.begin();

  tm.display("PLAY");

  // MIDI.begin(MIDI_CHANNEL_OMNI);

  //configure pin 2 as an input and enable the internal pull-up resistor
  // pinMode(1, INPUT_PULLUP);
  pinMode(3, INPUT_PULLUP);
  pinMode(4, INPUT_PULLUP);
  pinMode(5, INPUT_PULLUP);
  pinMode(6, INPUT_PULLUP);
  pinMode(7, INPUT_PULLUP);
  pinMode(8, INPUT_PULLUP);
  pinMode(9, INPUT_PULLUP);
  pinMode(10, INPUT_PULLUP);
  // pinMode(5, INPUT_PULLUP);
  // pinMode(6, INPUT_PULLUP);
  // pinMode(7, INPUT_PULLUP);
  // pinMode(8, INPUT_PULLUP);
  // pinMode(9, INPUT_PULLUP);
  // pinMode(10, INPUT_PULLUP);
  // pinMode(11, INPUT_PULLUP);
  // pinMode(12, INPUT_PULLUP);
  // pinMode(13, INPUT_PULLUP);

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, low);

}

void loop() {

  //read the pushbutton value into a variable
  int sensorVal1 = digitalRead(3);
  int sensorVal2 = digitalRead(4);
  int sensorVal3 = digitalRead(5);
  int sensorVal4 = digitalRead(6);
  int sensorVal5 = digitalRead(7);
  int sensorVal6 = digitalRead(8);
  int sensorVal7 = digitalRead(9);
  int sensorVal8 = digitalRead(10);

  if (sensorVal1 == LOW && !sensor1IsActive) {
      MIDI.sendNoteOn(60,127,2);
      sensor1IsActive = true;
      Serial.print(0);
      digitalWrite(LED_BUILTIN, HIGH);
  }
  if (sensorVal1 == HIGH && sensor1IsActive) {
      sensor1IsActive = false;
      MIDI.sendNoteOff(60,127,2);
      Serial.print(1);
      digitalWrite(LED_BUILTIN, LOW);
  }

  if (sensorVal2 == LOW) {
      MIDI.sendNoteOn(62,127,2);
  }
  if (sensorVal2 == HIGH) {
      // MIDI.sendNoteOff(61,127,2);
  }

  if (sensorVal3 == LOW) {
      MIDI.sendNoteOn(64,127,2);
  }
  if (sensorVal3 == HIGH) {
      // MIDI.sendNoteOff(62,127,2);
  }

  if (sensorVal4 == LOW) {
      MIDI.sendNoteOn(65,127,2);
  }
  if (sensorVal4 == HIGH) {
      // MIDI.sendNoteOff(63,127,2);
  }

  if (sensorVal5 == LOW) {
      MIDI.sendNoteOn(67,127,2);
      Serial.println(sensorVal5);
  }
  if (sensorVal5 == HIGH) {
      // MIDI.sendNoteOff(63,127,2);
  }

  if (sensorVal6 == LOW) {
      MIDI.sendNoteOn(69,127,2);
      Serial.println(sensorVal6);
  }
  if (sensorVal6 == HIGH) {
      // MIDI.sendNoteOff(63,127,2);
  }

  if (sensorVal7 == LOW) {
      MIDI.sendNoteOn(71,127,2);
      Serial.println(sensorVal7);
  }
  if (sensorVal7 == HIGH) {
      // MIDI.sendNoteOff(63,127,2);
  }

  if (sensorVal8 == LOW) {
      MIDI.sendNoteOn(72,127,2);
      Serial.println(sensorVal8);
  }
  if (sensorVal8 == HIGH) {
      // MIDI.sendNoteOff(63,127,2);
  }

  delay(100);

  // digitalWrite(LED_BUILTIN, LOW);
}
