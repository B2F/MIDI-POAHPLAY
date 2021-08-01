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

const unsigned long BAUD_RATE = 57600;

// Encoders:

const uint8_t NB_ENCODERS = 2;

const uint8_t P1CLK = 2;
const uint8_t P1DT = 4;
const uint8_t P1SW = 15;
const uint8_t P2CLK = 3;
const uint8_t P2DT = 5;
const uint8_t P2SW = 14;

Encoder P1(P1CLK, P1DT);
Encoder P2(P2CLK, P2DT);

Encoder encoder[NB_ENCODERS] = {P1, P2};
int encoderPos[NB_ENCODERS] = {0, 0};
int encoderState[NB_ENCODERS] = {1, 1};

const uint8_t ENCODER_STEP = 4;

// Multiplexer

const uint8_t MUXSIG = A0;
CD74HC4067 mux(A1, A2, A3, A4);

// MIDI

struct HairlessMidiSettings : public midi::DefaultSettings
{
   static const bool UseRunningStatus = false;
   static const long BaudRate = BAUD_RATE;
};

MIDI_CREATE_CUSTOM_INSTANCE(HardwareSerial, Serial, MIDI, HairlessMidiSettings);

byte midiCC1 = 0;
byte midiCC2 = 0;
byte midiCC1Value = 63;
byte midiCC2Value = 63;
byte midiChannel = 2;
byte globalVelocity = 127;
int globalNoteOffset = 0;
byte ticksPerNote = 96;
unsigned long startTime = 0;
long nbElapsedNotes = 0;
const int MIDI_START_OFFSET = 0;

char* midiNote[128] = {
  " C0 ",
  " CH0",
  " d0 ",
  " dH0",
  " E0 ",
  " F0 ",
  " FH0",
  " G0 ",
  " GH0",
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
  "A10 ",
  "AH10",
  "b10 ",
  "C10 ",
  "CH10",
  "d10 ",
  "dH10",
  "E10 ",
  "F10 ",
  "FH10",
  "G10 ",
};

// LCD

const uint8_t LCD_CLK = 12;
const uint8_t LCD_DIO = 11;

SevenSegmentFun display(LCD_CLK, LCD_DIO);

// Faders:

const uint8_t F1 = A6;
const uint8_t F2 = A7;

uint8_t faderPin[2] = {F1, F2};
int faderPos[2] = {0, 0};

const uint16_t MAX_FADER_VALUE = 970;
const uint16_t MIN_FADER_VALUE = 50;
const uint16_t FADER_THRESHOLD = 30;

// Leds:

const uint8_t L1 = 10;
const uint8_t L2 = 9;
const uint8_t L3 = 8;
const uint8_t L4 = 7;

// Ultrasonic

const uint8_t triggerPin = 13;
const uint8_t echoPin = 6;
UltraSonicDistanceSensor distanceSensor(triggerPin, echoPin);

// Magnet

const uint8_t MAGNET = A5;

// Switches

const uint8_t SW1 = 0;
const uint8_t SW2 = 1;
const uint8_t SW3 = 10;
const uint8_t SW4 = 11;

// @todo: ajouter un bouton reset qui peut aussi servir a verrouiller un note repeat.

bool midiCCIsActive = false;
bool ultrasonicSensorIsActive = false;
bool noteRepeatIsActive = false;
bool encoderSwitch1isActive = false;
bool encoderSwitch2isActive = false;
bool padSettingsLockIsActive = false;
bool padSettingsUnlockIsActive = false;
bool playButtonPressed = false;

// MIDI

#define MIDI_CLOCK 0xF8
#define MIDI_START 0xFA
#define MIDI_STOP 0xFC
#define MIDI_CONTINUE 0xFB
#define MIDI_SONG_POSITION_POINTER 0xF2
uint8_t currentVelocity = globalVelocity;
bool playFlag = false;
unsigned long midiCLockTick = 0;
unsigned long quarterNoteTime = 0;
uint8_t bpm = 120;
uint8_t repeatSpeedDividend = 1;
uint8_t repeatSpeedDivisor = 4;
uint8_t globalStartNote = 48;
uint16_t oneNoteTime = 0;
unsigned long stopTime = 0;

// Push buttons:

const byte PUSHED = LOW;
const byte RELEASED = HIGH;
const uint8_t NB_PUSH = 8;

uint8_t pushPin[NB_PUSH] = {4, 3, 2, 5, 6, 7, 8, 9};
// @todo Dynamic getPushNote setting relative to global if 0 and locked.
byte pushNote[NB_PUSH];
byte pushVelocity[NB_PUSH] = {100, 100, 100, 100, 100, 100, 100, 100};
bool pushSettingsLocked[NB_PUSH] = {false, false, false, false, false, false, false, false};
byte pushRepeatSpeed[NB_PUSH][2] = {{1,4}, {1,4}, {1,4}, {1,4}, {1,4}, {1,4}, {1,4}, {1,4}};
// Nb elapsed repeats timeframes (not necessarily used) since last start time:
unsigned long pushElapsedRepeats[NB_PUSH] = {0, 0, 0, 0, 0, 0, 0, 0};
byte isPushed[NB_PUSH] = {RELEASED, RELEASED, RELEASED, RELEASED, RELEASED, RELEASED, RELEASED, RELEASED};
int selectedPushPin = -1;

void setup() {

  Serial.begin(BAUD_RATE);

  readSwitches();

  // Push
  for (uint8_t pad = 0; pad < NB_PUSH; pad++) {
    pushNote[pad] = globalStartNote+pad;
  }

  // LCD:
  display.begin();
  if (!playButtonPressed) {
    display.print("P0AH");
    delay(1000);
    display.blink();
    display.print("P0AH PLAY");
    display.snake(2, 70);
  }

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
  oneNoteTime = getNoteMillis();

  // @todo init faders at setup.

}

void loop() {

  unsigned long loopTime = millis();

  if (updateMidiSerial()) {
    updateLedsTempo();
    playNotesRepeat();
  }
  else if (noteRepeatIsActive) {
    playNotesRepeat();
  }

  readSwitches();
  updatePads();

  // @todo afficher la valeur du fader 1 au démarrage.
  if (midiCCIsActive) {
    updateMidiControls();
  }
  else {

    uint8_t encoderValue = 127;
    uint16_t faderValue = 0;

    faderValue = readFader(0);
    if (faderValue != faderPos[0]) {
      faderPos[0] = faderValue;
      uint8_t newMidiValue = getMidiValueFromFader(faderPos[0]);
      updateVelocity(newMidiValue, newMidiValue);
    }

    faderValue = readFader(1);
    if (faderValue != faderPos[1]) {
      uint8_t newMidiValue = getMidiValueFromFader(faderPos[1]);
      MIDI.sendPitchBend(newMidiValue, midiChannel);
      displayPrintInt(newMidiValue);
      faderPos[1] = faderValue;
    }

    if (noteRepeatIsActive) {
      updateNoteRepeatSpeed();
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

  // double distance = distanceSensor.measureDistanceCm();
  // Serial.println(distance);
}

bool updateMidiSerial() {

  if (Serial.available()){
    return false;
  }

  unsigned long loopTime = millis();

  byte serialByte = Serial.read();

  if (serialByte == MIDI_CONTINUE) {
    playFlag = true;
    startTime += millis() - stopTime;
    return true;
  }
  if (serialByte == MIDI_START) {
    playFlag = true;
    startTime = loopTime - MIDI_START_OFFSET;
    nbElapsedNotes = 0;
    for (uint8_t pin = 0; pin < NB_PUSH; pin++) {
      pushElapsedRepeats[pin] = 0;
    }
    display.clear();
    display.print("PLAY");
    return true;
  }
  else if (serialByte == MIDI_STOP) {
    playFlag = false;
    stopTime = millis();
  }
  else if (serialByte == MIDI_SONG_POSITION_POINTER) {
    // @todo.
    byte serialData = Serial.read();
  }
  else if (serialByte == MIDI_CLOCK && playFlag) {

    MidiSync();

    if (loopTime > getNextNoteMillis()) {
      nbElapsedNotes++;
    }
    return true;
  }
  else {
    return false;
  }
}

void writeLeds(uint8_t s1, uint8_t s2, uint8_t s3, uint8_t s4) {
  uint8_t ledPin[4] = {L1, L2, L3, L4};
  uint8_t states[4] = {s1, s2, s3, s4};
  for (uint8_t i = 0; i < 4; i++) {
    digitalWrite(ledPin[i], states[i]);
  }
}

void playPush(uint8_t pin, bool state) {
  uint8_t currentVelocity = pushVelocity[pin];
  uint8_t currentNote = pushNote[pin];
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

uint8_t getMidiValueFromFader(uint16_t faderValue) {
  if (faderValue >= MAX_FADER_VALUE) {
    faderValue = 1024;
  }
  if (faderValue <= MIN_FADER_VALUE) {
    faderValue = 0;
  }
  return 127 - (faderValue * 127 / 1024);
}

float getPitchModulationFromfader(uint16_t faderValue) {
  if (faderValue > 512) {
    return (float) (faderValue - 512) / 512;
  }
  else {
    return (float) faderValue / 512 * -1;
  }
}

uint8_t getMidiValueFromEncoder(uint8_t currentMidiValue, int position, int previousPosition) {
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

char* getNoteFromMidiValue(uint8_t midiValue) {
  return midiNote[midiValue];
}

uint16_t readFader(uint8_t f) {
  uint16_t faderValue = analogRead(faderPin[f]);
  if (faderValue > faderPos[f] + FADER_THRESHOLD || faderValue < faderPos[f] - FADER_THRESHOLD) {
    if (faderValue >= MAX_FADER_VALUE) {
      return 1024;
    }
    else if (faderValue <= MIN_FADER_VALUE) {
      return 0;
    }
    return faderValue;
  }
  else {
    return faderPos[f];
  }
}

int readEncoder(uint8_t e) {
  int position = encoder[e].read();
  if (position != 0 && encoderPos[e] != position) {
    return position;
  }
  else {
    return encoderPos[e];
  }
}

void updateVelocity(uint8_t globalMidiValue, uint8_t localMidiValue) {
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
    if (pushNote[selectedPushPin] < 0) { pushNote[selectedPushPin] = 0; }
    char* note = getNoteFromMidiValue(pushNote[selectedPushPin]);
    displayPrintChar(note);
  }
  else {
    globalNoteOffset = globalMidiOffset;
    if (60+globalNoteOffset < 0) { globalNoteOffset = -60; }
    if (60+globalNoteOffset > 127) { globalNoteOffset = 67; }
    char* note = getNoteFromMidiValue(60+globalNoteOffset);
    displayPrintChar(note);
  }
}

void updatePads() {

  for (uint8_t p = 0; p < NB_PUSH; p++) {

    mux.channel(pushPin[p]);
    uint8_t sensorVal = digitalRead(MUXSIG);

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

      if (playFlag == false || !noteRepeatIsActive) {
        playPush(p, 1);
      }
      // @todo else start playback
    }
    else if (sensorVal == RELEASED && isPushed[p] == PUSHED) {
      isPushed[p] = RELEASED;
      playPush(p, 0);
    }
  }
}

void playNotesRepeat() {
  if (!noteRepeatIsActive) {
    return;
  }
  for (uint8_t pin = 0; pin < NB_PUSH; pin++) {
    unsigned long nextCap = getNextRepeatMillis(pin);
    if (millis() > nextCap) {
      pushElapsedRepeats[pin]++;
      if (isPushed[pin] == PUSHED) {
        playPush(pin, 1);
      }
    }
  }
}

float getPushPinFraction(uint8_t pin) {
  uint8_t currentRepeatSpeedDividend = pushSettingsLocked[pin] ? pushRepeatSpeed[pin][0] : repeatSpeedDividend;
  uint8_t currentRepeatSpeedDivisor = pushSettingsLocked[pin] ? pushRepeatSpeed[pin][1] : repeatSpeedDivisor;
  return (float) currentRepeatSpeedDividend / (float) currentRepeatSpeedDivisor;
} 

void updateLedsTempo() {

  unsigned long nextNoteMillis = getNextNoteMillis();
  unsigned long loopTime = millis();

  // @todo flash oxxx then oooo if no SPP found.
  if (loopTime > (nextNoteMillis - getOneNoteFractionMillis(0.25))) {
    writeLeds(HIGH, HIGH, HIGH, HIGH);
  }
  else if (loopTime > (nextNoteMillis - getOneNoteFractionMillis(0.5))) {
    writeLeds(HIGH, HIGH, HIGH, LOW);
  }
  else if (loopTime > (nextNoteMillis - getOneNoteFractionMillis(0.75))) {
    writeLeds(HIGH, HIGH, LOW, LOW);
  }
  else if (loopTime > (nextNoteMillis - oneNoteTime)) {
    writeLeds(HIGH, LOW, LOW, LOW);
  }

}

uint16_t getOneNoteFractionMillis(float fraction) {
  return oneNoteTime * fraction;
}

unsigned long getNextNoteMillis() {
  return startTime + ((nbElapsedNotes + 1) * oneNoteTime);
}

unsigned long getNextRepeatMillis(int pin) {
  float pinRepeatSpeed = getPushPinFraction(pin);
  uint16_t oneNoteFractionMillis = getOneNoteFractionMillis(pinRepeatSpeed);
  return startTime + (pushElapsedRepeats[pin] * oneNoteFractionMillis) + oneNoteFractionMillis;
}

uint16_t getNoteMillis() {
  return getBeatMillis(4);
}

uint16_t getBeatMillis(int nbBeats) {
  return (1000 / ((float) bpm / 60)) * (float) nbBeats;
}

void updateBpm() {

  // Quarter note time:
  if (midiCLockTick % 24 == 0) {

    // BPM:
    unsigned long bpmTime = millis();
    quarterNoteTime = bpmTime - quarterNoteTime;
    unsigned long newBpm = 60000/quarterNoteTime;
    if (bpm != newBpm && (bpm > newBpm + 1 || bpm < newBpm + 1)) {
      bpm = newBpm;
      displayPrintInt(bpm);
      oneNoteTime = getNoteMillis();
      startTime = startTime * (bpm / newBpm);
    }
    quarterNoteTime = bpmTime;
  }
}

void MidiSync() {
  updateBpm();
  midiCLockTick++;
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

  mux.channel(SW4);
  playButtonPressed = digitalRead(MUXSIG) == PUSHED ? true : false;

  mux.channel(P1SW);
  if (digitalRead(MUXSIG) == PUSHED) {
    padSettingsLockIsActive = true;
    padSettingsUnlockIsActive = false;
  }
  else {
    mux.channel(P2SW);
    padSettingsUnlockIsActive = digitalRead(MUXSIG) == PUSHED ? true : false;
    padSettingsLockIsActive = false;
  }
}

void updateMidiControls() {

  uint16_t faderValue = 127;
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

void updateNoteRepeatSpeed() {
  bool changed = false;
  int encoderValue = 0;
  uint8_t tmpRepeatSpeedDividend = repeatSpeedDividend;
  uint8_t tmpRepeatSpeedDivisor = repeatSpeedDivisor;
  encoderValue = readEncoder(0);
  tmpRepeatSpeedDividend = pushSettingsLocked[selectedPushPin] ? pushRepeatSpeed[selectedPushPin][0] : repeatSpeedDividend;
  tmpRepeatSpeedDivisor = pushSettingsLocked[selectedPushPin] ? pushRepeatSpeed[selectedPushPin][1] : repeatSpeedDivisor;
  if (encoderValue != encoderPos[0]) {
    tmpRepeatSpeedDividend = tmpRepeatSpeedDividend + encoderValue - encoderPos[0];
    if (tmpRepeatSpeedDividend > tmpRepeatSpeedDivisor) {
      tmpRepeatSpeedDividend = tmpRepeatSpeedDivisor;
    }
    if (tmpRepeatSpeedDividend < 1) { tmpRepeatSpeedDividend = 1; }
    encoderPos[0] = encoderValue;
    changed = true;
  }
  encoderValue = readEncoder(1);
  if (encoderValue != encoderPos[1]) {
    tmpRepeatSpeedDivisor = tmpRepeatSpeedDivisor + encoderValue - encoderPos[1];
    if (tmpRepeatSpeedDivisor > 32) {
      tmpRepeatSpeedDivisor = 32;
    }
    if (tmpRepeatSpeedDivisor < 1) { tmpRepeatSpeedDivisor = 1; }
    encoderPos[1] = encoderValue;
    changed = true;
  }
  if (changed) {

    if (pushSettingsLocked[selectedPushPin]) {
      pushRepeatSpeed[selectedPushPin][0] = tmpRepeatSpeedDividend;
      pushRepeatSpeed[selectedPushPin][1] = tmpRepeatSpeedDivisor;
    }
    else {
      repeatSpeedDividend = tmpRepeatSpeedDividend;
      repeatSpeedDivisor = tmpRepeatSpeedDivisor;
    }

    float newSpeedFraction = (float) tmpRepeatSpeedDividend / (float) tmpRepeatSpeedDivisor;
    unsigned long relativeStartTime = millis() - startTime;
    pushElapsedRepeats[selectedPushPin] = ceil((float) relativeStartTime / (float) getOneNoteFractionMillis(newSpeedFraction)) + 1;

    String dividend = tmpRepeatSpeedDividend >= 10 ? String(repeatSpeedDividend) : " " + String(repeatSpeedDividend);
    display.clear();
    display.print(dividend + String(tmpRepeatSpeedDivisor));
    display.setColonOn(true);
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


void displayPrintFloat(float f) {
  display.setColonOn(false);
  display.clear();
  display.print(f);
}