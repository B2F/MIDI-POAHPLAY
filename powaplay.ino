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
const int P1SW = 15;
const int P2CLK = 3;
const int P2DT = 5;
const int P2SW = 14;

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

int midiCC1 = 0;
int midiCC2 = 0;
int midiCC1Value = 63;
int midiCC2Value = 63;
int midiChannel = 2;
int globalVelocity = 127;
int globalNoteOffset = 60;
int ticksPerNote = 96;
int currentQuarter = 1;

char* midiNote[128] = {"","","","","","","","","","","","","","","","","","","","",
  " A0 ",
  " AH0",
  " b0 ",
  " C1 ",
  " CH1",
  " d1 ",
  " dH1",
  " E1 ",
  " F1 ",
  " FH1",
  " G1 ",
  " GH1",
  " A1 ",
  " AH1",
  " b1 ",
  " C2 ",
  " CH2",
  " d2 ",
  " dH2",
  " E2 ",
  " F2 ",
  " FH2",
  " G2 ",
  " GH2",
  " A2 ",
  " AH2",
  " b2 ",
  " C3 ",
  " CH3",
  " d3 ",
  " dH3",
  " E3 ",
  " F3 ",
  " FH3",
  " G3 ",
  " GH3",
  " A3 ",
  " AH3",
  " b3 ",
  " C4 ",
  " CH4",
  " d4 ",
  " dH4",
  " E4 ",
  " F4 ",
  " FH4",
  " G4 ",
  " GH4",
  " A4 ",
  " AH4",
  " b4 ",
  " C5 ",
  " CH5",
  " d5 ",
  " dH5",
  " E5 ",
  " F5 ",
  " FH5",
  " G5 ",
  " GH5",
  " A5 ",
  " AH5",
  " b5 ",
  " C6 ",
  " CH6",
  " d6 ",
  " dH6",
  " E6 ",
  " F6 ",
  " FH6",
  " G6 ",
  " GH6",
  " A6 ",
  " AH6",
  " b6 ",
  " C7 ",
  " CH7",
  " d7 ",
  " dH7",
  " E7 ",
  " F7 ",
  " FH7",
  " G7 ",
  " GH7",
  " A7 ",
  " AH7",
  " b7 ",
  " C8 ",
  " CH8",
  " d8 ",
  " dH8",
  " E8 ",
  " F8 ",
  " FH8",
  " G8 ",
  " GH8",
  " A8 ",
  " AH8",
  " b8 ",
  " C9 ",
  " CH9",
  " d9 ",
  " dH9",
  " E9 ",
  " F9 ",
  " FH9",
  " G9 ",
  " GH9",
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
int pushNote[NB_PUSH] = {60, 61, 62, 63, 64, 65, 66, 67};
int pushVelocity[NB_PUSH] = {100, 100, 100, 100, 100, 100, 100, 100};
int pushSettingsLocked[NB_PUSH] = {false, false, false, false, false, false, false, false};
int isPushed[NB_PUSH] = {RELEASED, RELEASED, RELEASED, RELEASED, RELEASED, RELEASED, RELEASED, RELEASED};
int selectedPushPin = -1;

// Faders:

const int F1 = A6;
const int F2 = A7;

int faderPin[2] = {F1, F2};
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

// @todo: ajouter un bouton reset qui peut aussi servir a verrouiller un note repeat.

bool midiCCIsActive = false;
bool ultrasonicSensorIsActive = false;
bool noteRepeatIsActive = false;
bool encoderSwitch1isActive = false;
bool encoderSwitch2isActive = false;
bool padSettingsLockIsActive = false;
bool padSettingsUnlockIsActive = false;

// MIDI

#define MIDI_CLOCK 0xF8
#define MIDI_START 0xFA
#define MIDI_STOP 0xFC
#define MIDI_CONTINUE 0xFB
int currentVelocity = globalVelocity;
int play_flag = 0;
int midiCLockTick = 0;
long int quarterNoteTime = 0;
int bpm = 120;
float noteRepeat;
int repeatSpeedDividend = 1;
int repeatSpeedDivisor = 4;

void setup() {

  Serial.begin(BAUD_RATE);

  // LCD:
  display.begin();
  display.print("P0AH");
  delay(1000);
  display.blink();
  display.print("P0AH PLAY");
  display.snake(2, 70);

  // Encoders:
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
    displayPrintString("PLAY");
  }
  else if (serialData == MIDI_STOP) {
    play_flag = 0;
  }
  else if ((serialData == MIDI_CLOCK) && (play_flag == 1)) {
    MidiSync();
  }

  readSwitches();

  if (midiCCIsActive) {
    updateMidiControls();
  }
  else {

    int encoderValue = 127;
    int faderValue = 0;

    faderValue = readFader(0);
    if (faderValue != faderPos[0]) {
      faderPos[0] = faderValue;
      int newMidiValue = getMidiValueFromFader(faderPos[0]);
      updateVelocity(newMidiValue, newMidiValue);
    }

    faderValue = readFader(1);
    if (faderValue != faderPos[1]) {
      int newMidiValue = getMidiValueFromFader(faderPos[1]);
      MIDI.sendPitchBend(newMidiValue, midiChannel);
      displayPrintInt(newMidiValue);
      faderPos[1] = faderValue;
    }

    if (noteRepeatIsActive) {
      bool changed = false;
      encoderValue = readEncoder(0);
      if (encoderValue != encoderPos[0]) {
        repeatSpeedDividend = repeatSpeedDividend + encoderValue - encoderPos[0];
        if (repeatSpeedDividend > repeatSpeedDivisor) {
          repeatSpeedDividend = repeatSpeedDivisor;
        }
        if (repeatSpeedDividend < 1) { repeatSpeedDividend = 1; }
        encoderPos[0] = encoderValue;
        changed = true;
      }
      encoderValue = readEncoder(1);
      if (encoderValue != encoderPos[1]) {
        repeatSpeedDivisor = repeatSpeedDivisor + encoderValue - encoderPos[1];
        if (repeatSpeedDivisor > 32) {
          repeatSpeedDivisor = 32;
        }
        if (repeatSpeedDivisor < 1) { repeatSpeedDivisor = 1; }
        encoderPos[1] = encoderValue;
        changed = true;
      }
      if (changed) {
        String dividend = repeatSpeedDividend >= 10 ? String(repeatSpeedDividend) : " " + String(repeatSpeedDividend);
        display.clear();
        display.print(dividend + String(repeatSpeedDivisor));
        display.setColonOn(true);
      }
    }
    else {
      encoderValue = readEncoder(0);
      if (encoderValue != encoderPos[0]) {
        updateVelocity(
          getMidiValueFromEncoder(globalVelocity, encoderValue, encoderPos[0]),
          getMidiValueFromEncoder(pushVelocity[selectedPushPin], encoderValue, encoderPos[0])
        );
        encoderPos[0] = encoderValue;
      }
      encoderValue = readEncoder(1);
      if (encoderValue != encoderPos[1]) {
        updateNotes(
          (getMidiValueFromEncoder(60+globalNoteOffset, encoderValue, encoderPos[1]) - 60),
          getMidiValueFromEncoder(pushNote[selectedPushPin], encoderValue, encoderPos[1])
        );
        encoderPos[1] = encoderValue;
      }
    }
  }

  updatePads();

  // double distance = distanceSensor.measureDistanceCm();
  // Serial.println(distance);
}

void writeLeds(int s1, int s2, int s3, int s4) {
  int ledPin[4] = {L1, L2, L3, L4};
  int states[4] = {s1, s2, s3, s4};
  for (int i = 0; i < 4; i++) {
    digitalWrite(ledPin[i], states[i]);
  }
}

void playPush(int pin, bool state) {
  int currentVelocity = pushVelocity[pin];
  int currentNote = pushNote[pin];
  if (!pushSettingsLocked[pin]) {
    currentVelocity = globalVelocity;
    currentNote += globalNoteOffset;
  }
  if (state) {
    MIDI.sendNoteOn(currentNote, currentVelocity, midiChannel);
    digitalWrite(LED_BUILTIN, HIGH);
    digitalWrite(MAGNET, HIGH);
  }
  else {
    MIDI.sendNoteOff(currentNote, currentVelocity, midiChannel);
    digitalWrite(LED_BUILTIN, LOW);
    digitalWrite(MAGNET, LOW);
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

long int getMidiValueFromEncoder(int currentMidiValue, int position, int previousPosition) {
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

char* getNoteFromMidiValue(int midiValue) {
  return midiNote[midiValue];
}

int readFader(int f) {
  int faderValue = analogRead(faderPin[f]);
  if (faderValue > faderPos[f] + FADER_THRESHOLD || faderValue < faderPos[f] - FADER_THRESHOLD) {
    return faderValue;
  }
  else {
    return faderPos[f];
  }
}

int readEncoder(int e) {
  int position = encoder[e].read();
  if (position != 0 && encoderPos[e] != position) {
    return position;
  }
  else {
    return encoderPos[e];
  }
}

void updateVelocity(int globalMidiValue, int localMidiValue) {
  if (selectedPushPin != -1 && pushSettingsLocked[selectedPushPin]) {
    pushVelocity[selectedPushPin] = localMidiValue;
    displayPrintInt(localMidiValue);
  }
  else {
    globalVelocity = globalMidiValue;
    pushVelocity[selectedPushPin] = globalVelocity;
    displayPrintInt(globalVelocity);
  }
}

void updateNotes(int globalMidiOffset, int localMidiOffset) {
  if (selectedPushPin != -1 && pushSettingsLocked[selectedPushPin]) {
    pushNote[selectedPushPin] = localMidiOffset;
    if (pushNote[selectedPushPin] < 20) {
      pushNote[selectedPushPin] = 20;
    }
    char* note = getNoteFromMidiValue(pushNote[selectedPushPin]);
    displayPrintChar(note);
  }
  else {
    globalNoteOffset = globalMidiOffset;
    if (60+globalNoteOffset < 20) { globalNoteOffset = -39; }
    if (60+globalNoteOffset > 119) { globalNoteOffset = 59; }
    char* note = getNoteFromMidiValue(60+globalNoteOffset);
    displayPrintChar(note);
  }
}

void updatePads() {

  for (int p = 0; p < NB_PUSH; p++) {

    mux.channel(pushPin[p]);
    int sensorVal = digitalRead(MUXSIG);

    if (sensorVal == PUSHED && isPushed[p] == RELEASED) {

      if (padSettingsLockIsActive && pushSettingsLocked[p] != true) {
        pushSettingsLocked[p] = true;
        displayPrintString("PAd" + String(p+1));
      }
      else if (padSettingsUnlockIsActive && pushSettingsLocked[p] != false) {
        pushSettingsLocked[p] = false;
        displayPrintString("Glob");
      }
      else if (selectedPushPin != p && pushSettingsLocked[p] == true) {
        displayPrintInt(pushVelocity[p]);
      }
      else if (selectedPushPin != p && pushSettingsLocked[p] == false) {
        displayPrintInt(globalVelocity);
      }

      isPushed[p] = PUSHED;
      selectedPushPin = p;

      playPush(p, 1);
    }
    if (sensorVal == RELEASED && isPushed[p] == PUSHED) {
      isPushed[p] = RELEASED;
      playPush(p, 0);
    }
  }
}

void MidiSync() {
  midiCLockTick++;
  if (noteRepeatIsActive) {
    float ticksPerBeat = ticksPerNote * repeatSpeedDividend / repeatSpeedDivisor;
    if (midiCLockTick == ticksPerBeat) {
      for (int pin = 0; pin < NB_PUSH; pin++) {
        if (isPushed[pin] == PUSHED) {
          playPush(pin, 1);
        }
      }
    }
    if (midiCLockTick == ticksPerBeat+1) {
      for (int pin = 0; pin < NB_PUSH; pin++) {
        if (isPushed[pin] == RELEASED) {
          playPush(pin, 0);
        }
      }
    }
  }
  if (midiCLockTick == 24) {
    // Tempo:
    if (currentQuarter > 4) {
      currentQuarter = 1;
    }
    if (currentQuarter == 1) {
      writeLeds(HIGH, LOW, LOW, LOW);
    }
    if (currentQuarter == 2) {
      writeLeds(HIGH, HIGH, LOW, LOW);
    }
    if (currentQuarter == 3) {
      writeLeds(HIGH, HIGH, HIGH, LOW);
    }
    if (currentQuarter == 4) {
      writeLeds(HIGH, HIGH, HIGH, HIGH);
    }
    currentQuarter++;

    // BPM:
    quarterNoteTime = millis() - quarterNoteTime;
    long int newBpm = 60000/quarterNoteTime;
    if (bpm != newBpm && (bpm > newBpm + 1 || bpm < newBpm + 1)) {
      bpm = newBpm;
      displayPrintInt(bpm);
    }
    midiCLockTick = 0;
    quarterNoteTime = millis();
  }
}

void readSwitches() {

  // @todo ajouter un mode ou le clic sur un encoder permet de modifier le midi channel en variant l'autre encoder et la vitesse de note repeat inversement.
  // en mode note repeat permet de changer la vitesse.
  // Ajouter un bouton reset des settings globaux.
  mux.channel(SW1);
  midiCCIsActive = digitalRead(MUXSIG);

  mux.channel(SW2);
  noteRepeatIsActive = digitalRead(MUXSIG);

  mux.channel(SW3);
  ultrasonicSensorIsActive = digitalRead(MUXSIG);

  mux.channel(P1SW);
  if (digitalRead(MUXSIG) == PUSHED) {
    padSettingsLockIsActive = true;
    padSettingsUnlockIsActive = false;
  }
  else {
    mux.channel(P2SW);
    if (digitalRead(MUXSIG) == PUSHED) {
      padSettingsUnlockIsActive = true;
    }
    else if (digitalRead(MUXSIG) == RELEASED) {
      padSettingsUnlockIsActive = false;
    }
    padSettingsLockIsActive = false;
  }
}

void updateMidiControls() {

  int faderValue = 127;
  int encoderValue = 0;

  if (!ultrasonicSensorIsActive) {
    // @todo Ajouter 8 raccourcis de midiCC via les pads lorsque les switch encoder sont sélectionnés.
    mux.channel(P1SW);
    if (digitalRead(MUXSIG) == PUSHED) {
      encoderValue = readEncoder(1);
      if (encoderValue != encoderPos[1]) {
        midiCC1 = getMidiValueFromEncoder(midiCC1, encoderValue, encoderPos[1]);
        encoderPos[1] = encoderValue;
        displayPrintString("C" + String(midiCC1));
      }
    }
    else {
      mux.channel(P2SW);
      if (digitalRead(MUXSIG) == PUSHED) {
        encoderValue = readEncoder(0);
        if (encoderValue != encoderPos[0]) {
          midiCC2 = getMidiValueFromEncoder(midiCC2, encoderValue, encoderPos[0]);
          displayPrintString("C" + String(midiCC2));
          encoderPos[0] = encoderValue;
        }
      }
    }
  }

  if (!noteRepeatIsActive) {
    encoderValue = readEncoder(0);
    if (encoderValue != encoderPos[0]) {
      midiCC1Value = getMidiValueFromEncoder(midiCC1Value, encoderValue, encoderPos[0]);
      MIDI.sendControlChange(midiCC1, midiCC1Value, midiChannel);
      encoderPos[0] = encoderValue;
      displayPrintInt(midiCC1Value);
    }
    encoderValue = readEncoder(1);
    if (encoderValue != encoderPos[1]) {
      midiCC2Value = getMidiValueFromEncoder(midiCC2Value, encoderValue, encoderPos[1]);
      MIDI.sendControlChange(midiCC2, midiCC2Value, midiChannel);
      encoderPos[1] = encoderValue;
      displayPrintInt(midiCC2Value);
    }
  }
  faderValue = readFader(0);
  if (faderValue != faderPos[0]) {
    faderPos[0] = faderValue;
    midiCC1Value = getMidiValueFromFader(faderPos[0]);
    MIDI.sendControlChange(midiCC1, midiCC1Value, midiChannel);
    displayPrintInt(midiCC1Value);
  }
  faderValue = readFader(1);
  if (faderValue != faderPos[1]) {
    faderPos[1] = faderValue;
    midiCC2Value = getMidiValueFromFader(faderPos[1]);
    MIDI.sendControlChange(midiCC2, midiCC2Value, midiChannel);
    displayPrintInt(midiCC2Value);
  }
}

void displayPrintString(String s) {
  display.setColonOn(false);
  display.clear();
  display.print(s);
}

void displayPrintInt(int i) {
  display.setColonOn(false);
  display.clear();
  display.print(i);
}

void displayPrintChar(char* c) {
  display.setColonOn(false);
  display.clear();
  display.print(c);
}