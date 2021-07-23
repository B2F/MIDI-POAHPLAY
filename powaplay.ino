/**
 * MIDI P0Ah PLAy
 */

#include "MIDI.h"
#include "SevenSegmentTM1637.h"
#include "SevenSegmentExtended.h"
#include "SevenSegmentFun.h"
#include <Encoder.h>
#include <light_CD74HC4067.h>
#include <HCSR04.h>

byte serialData;
const long BAUD_RATE = 57600;

// Encoders:

const int NB_ENCODERS = 2;

const int P1CLK = 2;
const int P1DT = 4;
const int P1SW = 14;
const int P2CLK = 3;
const int P2DT = 5;
const int P2SW = 15;

Encoder P1(P1CLK, P1DT);
Encoder P2(P2CLK, P2DT);

Encoder encoder[NB_ENCODERS] = {P1, P2};
int encoderPos[NB_ENCODERS] = {0, 0};
int encoderState[NB_ENCODERS] = {1, 1};

const int ENCODER_STEP = 4;

// Multiplexer

const int MUXSIG = A0;
CD74HC4067 mux(A1, A2, A3, A4);

// MIDI

struct HairlessMidiSettings : public midi::DefaultSettings
{
   static const bool UseRunningStatus = false;
   static const long BaudRate = BAUD_RATE;
};

MIDI_CREATE_CUSTOM_INSTANCE(HardwareSerial, Serial, MIDI, HairlessMidiSettings);

int midiChannel = 2;
int globalVelocity = 127;
int globalNote = 60;

String midiNote[128] = {"","","","","","","","","","","","","","","","","","","","",
  "A0",
  "A#0",
  "B0",
  "C1",
  "C#1",
  "D1",
  "D#1",
  "E1",
  "F1",
  "F#1",
  "G1",
  "G#1",
  "A1",
  "A#1",
  "B1",
  "C2",
  "C#2",
  "D2"
  "D#2",
  "E2",
  "F2",
  "F#2",
  "G2",
  "G#2",
  "A2",
  "A#2",
  "B2",
  "C3",
  "C#3",
  "D3"
  "D#3",
  "E3",
  "F3",
  "F#3",
  "G3",
  "G#3",
  "A3",
  "A#3",
  "B3",
  "C4",
  "C#4",
  "D4"
  "D#4",
  "E4",
  "F4",
  "F#4",
  "G4",
  "G#4",
  "A4",
  "A#4",
  "B4",
  "C5",
  "C#5",
  "D5"
  "D#5",
  "E5",
  "F5",
  "F#5",
  "G5",
  "G#5",
  "A5",
  "A#5",
  "B5",
  "C6",
  "C#6",
  "D6"
  "D#6",
  "E6",
  "F6",
  "F#6",
  "G6",
  "G#6",
  "A6",
  "A#6",
  "B6",
  "C7",
  "C#7",
  "D7"
  "D#7",
  "E7",
  "F7",
  "F#7",
  "G7",
  "G#7",
  "A7",
  "A#7",
  "B7",
  "C8",
  "C#8",
  "D8"
  "D#8",
  "E8",
  "F8",
  "F#8",
  "G8",
  "G#8",
  "A8",
  "A#8",
  "B8",
  "C9",
  "C#9",
  "D9"
  "D#9",
  "E9",
  "F9",
  "F#9",
  "G9",
  "G#9",
};

// LCD

const int LCD_CLK = 12;
const int LCD_DIO = 11;

SevenSegmentFun display(LCD_CLK, LCD_DIO);

// Push buttons:

const int PUSHED = LOW;
const int RELEASED = HIGH;
const int NB_PUSH = 8;

int pushPin[NB_PUSH] = {4, 3, 2, 5, 6, 7, 8, 9};
int pushNote[NB_PUSH] = {60, 62, 63, 64, 65, 67, 71, 72};
int pushVelocity[NB_PUSH] = {100, 100, 100, 100, 100, 100, 100, 100};
int isPushed[NB_PUSH] = {RELEASED, RELEASED, RELEASED, RELEASED, RELEASED, RELEASED, RELEASED, RELEASED};
int selectedPushPin = -1;

// Faders:

const int F1 = A6;
const int F2 = A7;

int faderPos[2] = {0, 0};

const int MAX_FADER_VALUE = 970;
const int MIN_FADER_VALUE = 50;
const int FADER_THRESHOLD = 30;

// Leds:

const int L1 = 10;
const int L2 = 9;
const int L3 = 8;
const int L4 = 7;

// Ultrasonic

const int triggerPin = 13;
const int echoPin = 6;
UltraSonicDistanceSensor distanceSensor(triggerPin, echoPin);

// Magnet

const int MAGNET = A5;

// Switches

const int SW1 = 0;
const int SW2 = 1;
const int SW3 = 10;
const int SW4 = 13;

// MIDI Clock

#define MIDI_CLOCK 0xF8
#define MIDI_START 0xFA
#define MIDI_STOP 0xFC
#define MIDI_CONTINUE 0xFB
int play_flag = 0;
int midiCLockTick = 0;
long int quarterNoteTime = 0;

void setup() {

  Serial.begin(BAUD_RATE);

  display.begin();
  // display.setBacklight(50);
  // byte repeats = 1;
  // display.scrollingText("P0AH PLAY", repeats);
  // delay(1000);   

  pinMode(P1CLK, INPUT);
  pinMode(P1DT, INPUT);
  pinMode(P2CLK, INPUT);
  pinMode(P2DT, INPUT);

  // Leds:
  pinMode(L1, OUTPUT);
  pinMode(L2, OUTPUT);
  pinMode(L3, OUTPUT);
  pinMode(L4, OUTPUT);

  // Magnet:
  pinMode(MAGNET, OUTPUT);

  // mux sig:
  pinMode(A0, INPUT_PULLUP);

  // Internal led:
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  // MIDI Clock:
  quarterNoteTime = millis();
}

void loop() {

  serialData = Serial.read();
  if (serialData == MIDI_START || serialData == MIDI_CONTINUE) {
    play_flag = 1;
    display.clear();
    display.print("PLAY");
  }
  else if (serialData == MIDI_STOP) {
    play_flag = 0;
  }
  else if ((serialData == MIDI_CLOCK) && (play_flag == 1)) {
    Sync();
  }

  mux.channel(SW1);
  bool midiCCIsActive = digitalRead(MUXSIG);

  mux.channel(SW2);
  bool globalPosIsActive = digitalRead(MUXSIG);

  mux.channel(SW3);
  bool ultrasonicSensorIsActive = digitalRead(MUXSIG);

  mux.channel(SW4);
  bool noteRepeatIsActive = digitalRead(MUXSIG);

  int faderValue = analogRead(F1);
  if (faderValue > faderPos[0] + FADER_THRESHOLD || faderValue < faderPos[0] - FADER_THRESHOLD) {
    display.clear();
    if (globalPosIsActive) {
      globalVelocity = getMidiValueFromFader(faderValue);
      display.print(globalVelocity);
    }
    else if (selectedPushPin != -1) {
      pushVelocity[selectedPushPin] = getMidiValueFromFader(faderValue);
      display.print(pushVelocity[selectedPushPin]);
    }
    faderPos[0] = faderValue;
  }

  // faderValue = getMidiValueFromFader(analogRead(F2));
  // if (faderValue != faderPos[1]){
  //   faderPos[1] = faderValue;
  // }

  // @todo ajouter un mode ou le clic sur un encoder permet de modifier le midi channel en variant l'autre encoder et la vitesse de note repeat inversement.

  int position = P1.read();
  if (position != 0 && encoderPos[0] != position) {
     display.clear();
     if (globalPosIsActive) {
      globalVelocity = getMidiValueFromEncoder(globalVelocity, position, encoderPos[0]);
      display.print(globalVelocity);
    }
    else if (selectedPushPin != -1) {
      pushVelocity[selectedPushPin] = getMidiValueFromEncoder(pushVelocity[selectedPushPin], position, encoderPos[0]);
      display.print(pushVelocity[selectedPushPin]);
    }
    encoderPos[0] = position;
  }

  // position = P2.read();
  // if (position != 0 && encoderPos[1] != position) {
  //   encoderPos[1] = position;
  //   // tm.clearScreen();
  //   // tm.display(String(position));
  // }

  for (int p = 0; p < NB_PUSH; p++) {

    mux.channel(pushPin[p]);
    int sensorVal = digitalRead(MUXSIG);

    int currentVelocity = pushVelocity[p];
    if (globalPosIsActive) {
      currentVelocity = globalVelocity;
    }

    if (sensorVal == PUSHED && isPushed[p] == RELEASED) {
        MIDI.sendNoteOn(pushNote[p], currentVelocity, midiChannel);
        isPushed[p] = PUSHED;
        digitalWrite(LED_BUILTIN, HIGH);
        selectedPushPin = p;
        if (!globalPosIsActive) {
          display.clear();
          display.print(currentVelocity);
        }
    }
    if (sensorVal == RELEASED && isPushed[p] == PUSHED) {
        isPushed[p] = RELEASED;
        MIDI.sendNoteOff(pushNote[p], currentVelocity, midiChannel);
        digitalWrite(LED_BUILTIN, LOW);
    }
  }

  // double distance = distanceSensor.measureDistanceCm();
  // Serial.println(distance);

  if (globalPosIsActive) {
    digitalWrite(L1, HIGH);
    digitalWrite(L2, HIGH);
    digitalWrite(L3, HIGH);
    digitalWrite(L4, HIGH);
  }
  else {
    switch (selectedPushPin) {
      case 0:
        digitalWrite(L1, HIGH);
        digitalWrite(L2, HIGH);
        digitalWrite(L3, LOW);
        digitalWrite(L4, LOW);
        break;
      case 1:
        digitalWrite(L1, HIGH);
        digitalWrite(L2, LOW);
        digitalWrite(L3, HIGH);
        digitalWrite(L4, LOW);
        break;
      case 2:
        digitalWrite(L1, LOW);
        digitalWrite(L2, HIGH);
        digitalWrite(L3, LOW);
        digitalWrite(L4, HIGH);
        break;
      case 3:
        digitalWrite(L1, LOW);
        digitalWrite(L2, LOW);
        digitalWrite(L3, HIGH);
        digitalWrite(L4, HIGH);
        break;
      case 4:
        digitalWrite(L1, HIGH);
        digitalWrite(L2, LOW);
        digitalWrite(L3, LOW);
        digitalWrite(L4, LOW);
        break;
      case 5:
        digitalWrite(L1, LOW);
        digitalWrite(L2, HIGH);
        digitalWrite(L3, LOW);
        digitalWrite(L4, LOW);
        break;
      case 6:
        digitalWrite(L1, LOW);
        digitalWrite(L2, LOW);
        digitalWrite(L3, HIGH);
        digitalWrite(L4, LOW);
        break;
      case 7:
        digitalWrite(L1, LOW);
        digitalWrite(L2, LOW);
        digitalWrite(L3, LOW);
        digitalWrite(L4, HIGH);
        break;
    }
  }
}

long int getMidiValueFromFader(long int faderValue) {
  if (faderValue >= MAX_FADER_VALUE) {
    faderValue = 1024;
  }
  if (faderValue <= MIN_FADER_VALUE) {
    faderValue = 0;
  }
  return 127 - (faderValue * 127 / 1024);
}

int getMidiValueFromEncoder(int currentMidiValue, int position, int previousPosition) {
  int delta = position - previousPosition;
  int newMidiValue = currentMidiValue + delta;
  if (newMidiValue >= 127) {
    return 127;
  }
  else if (newMidiValue <= 0) {
    return 0;
  }
  return newMidiValue;
}

String getNoteFromFaderValue(int faderValue) {
  return midiNote[getMidiValueFromFader(faderValue)];
}

void Sync() {
  midiCLockTick++;
  if (midiCLockTick == 24) {
    quarterNoteTime = millis() - quarterNoteTime;
    long int bpm = 60000/quarterNoteTime;
    display.clear();
    display.print(bpm);
    midiCLockTick = 0;
    quarterNoteTime = millis();
  }
}
