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

const unsigned long BAUD_RATE = 38400;

// Misc

const byte UNASSIGNED = 255;

// Push buttons:

const byte PUSHED = LOW;
const byte RELEASED = HIGH;
const byte NB_PUSH = 8;
// Including NOTE (no chord).
const byte NB_CHORDS = 4;
const byte MAX_NOTES = 7;

// Encoders:

const byte NB_ENCODERS = 2;

const byte P1CLK = 2;
const byte P1DT = 4;
const byte P1SW = 15;
const byte P2CLK = 3;
const byte P2DT = 5;
const byte P2SW = 14;

Encoder P1(P1CLK, P1DT);
Encoder P2(P2CLK, P2DT);

Encoder encoder[NB_ENCODERS] = {P1, P2};
// Old state:
int encoderPos[NB_ENCODERS] = {0, 0};
// New state:
int encoderVal[NB_ENCODERS] = {0, 0};

const byte ENCODER_STEP = 4;

// Multiplexer

const byte MUXSIG = A0;
CD74HC4067 mux(A1, A2, A3, A4);

// MIDI

struct HairlessMidiSettings : public midi::DefaultSettings
{
   static const bool UseRunningStatus = false;
   static const long BaudRate = BAUD_RATE;
};

MIDI_CREATE_CUSTOM_INSTANCE(HardwareSerial, Serial, MIDI, HairlessMidiSettings);

byte midiCC[2] = {91, 92};
byte midiCCValue[2] = {63, 63};
// https://professionalcomposers.com/midi-cc-list/
// 5, 7, 10, 71, 72, 73, 74, 80, 81, 84, 91, 92, 93, 94, 95 - 98-101.
byte midiCCPresets[NB_PUSH] = {91, 92, 93, 94, 95, 98, 99, 100};
byte midiChannel = 2;
byte programChange = 0;
byte globalVelocity = 127;
int globalNoteOffset = 0;
byte ticksPerNote = 96;
unsigned long startTime = 0;
long nbElapsedNotes = 0;
const int MIDI_START_OFFSET = 0;
unsigned long lastClockPulse = 0;
unsigned long lastNoteRepeat = 0;

char* midiNote[128] = {
  " C0 ", " d0b", " d0 ", " E0b", " E0 ", " F0 ", " F0b", " G0 ", " A0b", " A0 ", " b0b", " b0 ",
  " C1 ", " d1b", " d1 ", " E1b", " E1 ", " F1 ", " F1b", " G1 ", " A1b", " A1 ", " b1b", " b1 ",
  " C2 ", " d2b", " d2 ", " E2b", " E2 ", " F2 ", " F2b", " G2 ", " A2b", " A2 ", " b2b", " b2 ",
  " C3 ", " d3b", " d3 ", " E3b", " E3 ", " F3 ", " F3b", " G3 ", " A3b", " A3 ", " b3b", " b3 ",
  " C4 ", " d4b", " d4 ", " E4b", " E4 ", " F4 ", " F4b", " G4 ", " A4b", " A4 ", " b4b", " b4 ",
  " C5 ", " d5b", " d5 ", " E5b", " E5 ", " F5 ", " F5b", " G5 ", " A5b", " A5 ", " b5b", " b5 ",
  " C6 ", " d6b", " d6 ", " E6b", " E6 ", " F6 ", " F6b", " G6 ", " A6b", " A6 ", " b6b", " b6 ",
  " C7 ", " d7b", " d7 ", " E7b", " E7 ", " F7 ", " F7b", " G7 ", " A7b", " A7 ", " b7b", " b7 ",
  " C8 ", " d8b", " d8 ", " E8b", " E8 ", " F8 ", " F8b", " G8 ", " A8b", " A8 ", " b8b", " b8 ",
  " C9 ", " d9b", " d9 ", " E9b", " E9 ", " F9 ", " G9b", " G9 ", " A9b", "A10 ", "b10b", "b10 ",
  "C10 ", "d10b", "d10 ", "E0b", "E10 ", "F10 ", "G10b", "G10 ",
};

// LCD

const byte LCD_CLK = 12;
const byte LCD_DIO = 11;

SevenSegmentFun display(LCD_CLK, LCD_DIO);

// Faders:

const byte F1 = A6;
const byte F2 = A7;

byte faderPin[2] = {F1, F2};
const byte NB_FADERS = 2;
uint16_t faderPos[NB_FADERS] = {0, 0};
uint16_t faderVal[NB_FADERS] = {0, 0};

const uint16_t MAX_FADER_VALUE = 970;
const uint16_t MIN_FADER_VALUE = 50;
const uint16_t FADER_THRESHOLD = 30;

// Leds:

const byte L1 = 10;
const byte L2 = 9;
const byte L3 = 8;
const byte L4 = 7;

// Ultrasonic

const byte triggerPin = 13;
const byte echoPin = 6;

UltraSonicDistanceSensor distanceSensor(triggerPin, echoPin);

byte MIN_ULTRASONIC_DISTANCE_CM = 5;
byte maxUltrasonicDistanceCm = 10;
byte ultrasonicCC = 100;

// Magnet

const byte MAGNET = A5;

// Switches

const byte SW_CC = 0;
const byte SW_REPEAT = 1;
const byte SW_ULTRASONIC = 10;
const byte SW_PLAY = 11;

const byte INIT_MASK =       B00000010;
const byte CC_MASK =         B00000100;
const byte REPEAT_MASK =     B00001000;
const byte ULTRASONIC_MASK = B00010000;

byte currentPlayMode = B00000000;

byte switches[4][2] = {
  {SW_CC, CC_MASK},
  {SW_PLAY, INIT_MASK},
  {SW_ULTRASONIC, ULTRASONIC_MASK},
  {SW_REPEAT, REPEAT_MASK}
};

// @todo: ajouter un bouton save (sd card).
// @todo: ajouter un bouton pour voir les réglages (velo, note, cc) dans le lcd.

bool midiCCIsActive = false;
bool ultrasonicSensorIsActive = false;
bool noteRepeatIsActive = false;
bool encoderSwitch1isActive = false;
bool encoderSwitch2isActive = false;
byte rightPush = RELEASED;
byte leftPush = RELEASED;
bool initButtonPressed = false;

// MIDI

#define MIDI_CLOCK 0xF8
#define MIDI_START 0xFA
#define MIDI_STOP 0xFC
#define MIDI_CONTINUE 0xFB
#define MIDI_SONG_POSITION_POINTER 0xF2
byte currentVelocity = globalVelocity;
bool playFlag = false;
unsigned long midiCLockTick = 0;
unsigned long quarterNoteTime = 0;
byte bpm = 120;
byte repeatSpeedDividend = 1;
byte repeatSpeedDivisor = 4;
byte globalStartNote = 48;
unsigned long oneNoteTime = 0;
unsigned long stopTime = 0;
int octave = 0;

byte pushPin[NB_PUSH] = {4, 3, 2, 5, 6, 7, 8, 9};
byte pushNote[NB_PUSH];
byte pushVelocity[NB_PUSH] = {100, 100, 100, 100, 100, 100, 100, 100};
bool pushSettingsLocked[NB_PUSH] = {false, false, false, false, false, false, false, false};
byte pushRepeatSpeed[NB_PUSH][2] = {{1,4}, {1,4}, {1,4}, {1,4}, {1,4}, {1,4}, {1,4}, {1,4}};
// Nb elapsed repeats timeframes (not necessarily used) since last start time:
unsigned long pushElapsedRepeats[NB_PUSH] = {0, 0, 0, 0, 0, 0, 0, 0};
byte isPushed[NB_PUSH] = {
  RELEASED,
  RELEASED,
  RELEASED,
  RELEASED,
  RELEASED,
  RELEASED,
  RELEASED,
  RELEASED
};
unsigned long pushedTime[NB_PUSH] = {0,0,0,0,0,0,0,0};
bool repeatIsLocked[NB_PUSH] = {false, false, false, false, false, false, false, false};
int selectedPushPin = -1;

// Chords
// @see https://spinditty.com/learning/chord-building-for-musicians
byte chords[NB_CHORDS][MAX_NOTES] = {
  {0, UNASSIGNED, UNASSIGNED, UNASSIGNED, UNASSIGNED, UNASSIGNED, UNASSIGNED},
  {0, 4, 7, UNASSIGNED, UNASSIGNED, UNASSIGNED, UNASSIGNED},
  {0, 3, 7, UNASSIGNED, UNASSIGNED, UNASSIGNED, UNASSIGNED},
  {0, 4, 8, UNASSIGNED, UNASSIGNED, UNASSIGNED, UNASSIGNED},
};
char* chordsNames[NB_CHORDS] = {
  // NOTE <=> no chord.
  "NOTE",
  "MAJ",
  "MIN",
  "AUG"
};
byte selectedChord = 0;

void reinit() {
  midiCC[0] = 0;
  midiCC[1] = 0;
  midiCCValue[0] = 63;
  midiCCValue[1] = 63;
  // https://professionalcomposers.com/midi-cc-list/
  // 5, 7, 10, 71, 72, 73, 74, 80, 81, 84, 91, 92, 93, 94, 95 - 98-101.
  midiCCPresets[0] = 91;
  midiCCPresets[1] = 92;
  midiCCPresets[2] = 93;
  midiCCPresets[3] = 94;
  midiCCPresets[4] = 95;
  midiCCPresets[5] = 98;
  midiCCPresets[6] = 99;
  midiCCPresets[7] = 100;
  midiChannel = 2;
  programChange = 0;
  globalVelocity = 127;
  globalNoteOffset = 0;
  ultrasonicCC = 100;
  currentPlayMode = B00000000;
  midiCCIsActive = false;
  ultrasonicSensorIsActive = false;
  noteRepeatIsActive = false;
  encoderSwitch1isActive = false;
  encoderSwitch2isActive = false;
  rightPush = RELEASED;
  leftPush = RELEASED;
  initButtonPressed = false;
  repeatSpeedDividend = 1;
  repeatSpeedDivisor = 4;
  globalStartNote = 48;

  for (byte i = 0; i < 8; i++) {
    pushVelocity[i] = 100;
  }
  for (byte i = 0; i < 8; i++) {
    pushSettingsLocked[i] = false;
  }
  for (byte i = 0; i < 8; i++) {
    pushRepeatSpeed[i][0] = 1;
    pushRepeatSpeed[i][1] = 4;
  }
  for (byte i = 0; i < 8; i++) {
    pushElapsedRepeats[i] = 0;
  }
  for (byte i = 0; i < 8; i++) {
    isPushed[i] = RELEASED;
  }
  for (byte i = 0; i < 8; i++) {
    repeatIsLocked[i] = false;
  }
  selectedPushPin = -1;
  octave = 0;
  selectedChord = 0;
}

void setup() {

  Serial.begin(BAUD_RATE);
  while(!Serial) ;

  Serial.print("bnobs.art P0AH PLAY --- Compiled on ");
  Serial.print(__DATE__);
  Serial.print(" at ");
  Serial.println(__TIME__);
  Serial.println();

  // Push
  for (byte pad = 0; pad < NB_PUSH; pad++) {
    pushNote[pad] = globalStartNote+pad;
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
  pinMode(MUXSIG, INPUT_PULLUP);

  // Internal led:
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  // MIDI Clock:
  quarterNoteTime = micros();
  oneNoteTime = getNoteMicros();

  readSwitches();

  // LCD:
  display.begin();
  display.print("P0AH");
  delay(1000);
  if (checkMode(INIT_MASK)) {
    readSwitches();
    if (checkMode(INIT_MASK)) {
      display.blink();
      readSwitches();
      if (checkMode(INIT_MASK)) {
        display.print("P0AH PLAY");
        readSwitches();
        if (checkMode(INIT_MASK)) {
          display.snake(2, 70);
        }
      }
    }
  }
}

void testFaders() {
  if (faderPos[0] != faderVal[0]) {
    faderPos[0] = faderVal[0];
    Serial.println(faderVal[0]);
    Serial.println("Selected 1");
  }
  if (faderPos[1] != faderVal[1]) {
    faderPos[1] = faderVal[1];
    Serial.println(faderVal[1]);
    Serial.println("Selected 2");
  }
}

void loop() {

  unsigned long loopTime = micros();

  updateMidiSerial();
  if (playFlag) {
    updateLedsTempo();
  }
  else if (playFlag == false) {
    playNotesRepeat();
  }

  readSwitches();
  updatePads();

  for (byte pos = 0; pos < NB_FADERS; pos++) {
    faderVal[pos] = readFader(pos);
  }
  for (byte pos = 0; pos < NB_ENCODERS; pos++) {
    encoderVal[pos] = readEncoder(pos); 
  }

  if (!checkMode(INIT_MASK) && !initButtonPressed) {
    initButtonPressed = true;
    if (leftPush == PUSHED) {
      displayPrint("init", false, true);
      reinit();
    }
  }
  else if (checkMode(INIT_MASK) && initButtonPressed == true) {
    initButtonPressed = false;
  }

  // Init button
  if (initButtonPressed == true) {
    updateChannelFromEncoder(0);
    updateBaseNoteFromEncoder(1);
    return;
  }

  // Encoders et faders (sans encoder push)
  if (checkMode(CC_MASK)) {
    updateCCValueFromFader(0);
    updateCCValueFromFader(1);
    if (rightPush == RELEASED) {
      updateMidiCCValueFromEncoder(0);
    }
    if (leftPush == RELEASED) {
      updateMidiCCValueFromEncoder(1);
    }
  }
  else {
    updateVelocityFromFader(0);
    updateOctaveFromFader(1);
    if (rightPush == RELEASED) {
      updateVelocityFromEncoder(0);
    }
    if (leftPush == RELEASED) {
      updateOctaveFromEncoder(1);
    }
  }

  // Encoder push
  if (checkMode(ULTRASONIC_MASK)) {
    if (rightPush == PUSHED) {
      updateUltrasonicCC(0);
    }
    if (leftPush == PUSHED) {
      updateUltrasonicDistance(1);
    }
  }
  else if (checkMode(REPEAT_MASK)) {
    if (leftPush == PUSHED) {
      updateNoteRepeatSpeedFromEncoder(1);
      updatePadsRepeatLockUnlock(true);
    }
    if (rightPush == PUSHED) {
      // @todo updateRepeatOctaveUpDown
      updatePadsRepeatLockUnlock(false);
    }
  }
  else if (checkMode(CC_MASK)) {
    if (rightPush == PUSHED) {
      updateMidiControlFromEncoder(0);
      selectCCPreset(0);
    }
    if (leftPush == PUSHED) {
      updateMidiControlFromEncoder(1);
      selectCCPreset(1);
    }
  }
  else {
    // chords and scales
    if (rightPush == PUSHED) {
      updatePadsLock(true);
    }
    if (leftPush == PUSHED) {
      chordAndScaleSelect(1);
      updatePadsLock(false);
    }
  }

  if (checkMode(ULTRASONIC_MASK)) {
    trackUltrasonicChanges();
  }
}

void updateChannelFromEncoder(byte selected) {
  if (encoderVal[selected] != encoderPos[selected]) {
    int newMidiChannel = getMidiValueFromEncoder(
      midiChannel,
      encoderVal[selected],
      encoderPos[selected]
    );
    midiChannel = newMidiChannel > 15 ? 16 : newMidiChannel;
    display.clear();
    display.setColonOn(false);
    display.print("CH" + String(midiChannel));
    encoderPos[selected] = encoderVal[selected];
  }
}

void updateBaseNoteFromEncoder(byte selected) {
  if (encoderVal[selected] != encoderPos[selected]) {
    updateNotes(
      (getMidiValueFromEncoder(60+globalNoteOffset, encoderVal[selected], encoderPos[selected]) - 60),
      getMidiValueFromEncoder(pushNote[selectedPushPin], encoderVal[selected], encoderPos[selected])
    );
  }
  encoderPos[selected] = encoderVal[selected];
}

void chordAndScaleSelect(byte selected) {

  encoderVal[selected] = readEncoder(selected);
  if (encoderVal[selected] == encoderPos[selected]) {
    return;
  }

  // Chord:
  if (encoderVal[selected] > encoderPos[selected]) {
    selectedChord = selectedChord < (NB_CHORDS-1) ? selectedChord+1 : selectedChord;
  }
  else if (selectedChord > 0) {
    selectedChord = selectedChord > 0 ? selectedChord - 1 : 0;
  }
  display.clear();
  display.print(chordsNames[selectedChord]);
  display.setColonOn(false);
  encoderPos[selected] = encoderVal[selected];
}

void updateOctaveFromFader(byte selected) {
  if (faderVal[selected] == faderPos[selected]) {
    return;
  }
  int relativeValue = (faderVal[selected] - 1024) * (-1);
  if (relativeValue >= MAX_FADER_VALUE) {
    relativeValue = 1024;
  }
  if (relativeValue <= MIN_FADER_VALUE) {
    relativeValue = 0;
  }
  octave = round(relativeValue / 100) - 4;
  display.clear();
  if (octave >= 0) {
    display.print(String(octave) + "oct");
  }
  else {
    display.print(String(octave) + "oc");
  }
  globalNoteOffset = octave * 12;
  faderPos[selected] = faderVal[selected];
}

void updateVelocityFromEncoder(byte selected) { 
  if (encoderVal[selected] == encoderPos[selected]) {
    return;
  }
  updateVelocity(
    getMidiValueFromEncoder(globalVelocity, encoderVal[selected], encoderPos[selected]),
    getMidiValueFromEncoder(pushVelocity[selectedPushPin], encoderVal[selected], encoderPos[selected])
  );
  encoderPos[selected] = encoderVal[selected];
}

void updateOctaveFromEncoder(byte selected) {
  if (encoderVal[selected] == encoderPos[selected]) {
    return;
  }
  bool direction = encoderVal[selected] > encoderPos[selected];
  moveOctave(direction);
  encoderPos[selected] = encoderVal[selected];
}

bool updateMidiSerial() {

  if (!Serial.available()){
    return false;
  }

  unsigned long loopTime = micros();

  byte serialByte = Serial.read();

  if (serialByte == MIDI_CONTINUE) {
    playFlag = true;
    startTime += micros() - stopTime;
    displayPrint("CONT", false, true);
  }
  if (serialByte == MIDI_START) {
    playFlag = true;
    startTime = loopTime - MIDI_START_OFFSET;
    nbElapsedNotes = 0;
    displayPrint("PLAY", false, true);
  }
  if (serialByte == MIDI_STOP) {
    playFlag = false;
    stopTime = micros();
    displayPrint("STOP", false, true);
  }
  if (serialByte == MIDI_SONG_POSITION_POINTER) {
    // @todo.
    displayPrint("POS", false, true);
    // serialByte = Serial.read();
    // byte serialByte2 = Serial.read();
    // delay(1000);
    // display.clear();
    // display.println((int) serialByte);
    // delay(2000);
  }
  if (serialByte == MIDI_CLOCK && playFlag) {

    MidiSync();

    if (loopTime > getNextNoteMicros()) {
      nbElapsedNotes++;
    }

    playNotesRepeat();
  }
}

void writeLeds(byte s1, byte s2, byte s3, byte s4) {
  byte ledPin[4] = {L1, L2, L3, L4};
  byte states[4] = {s1, s2, s3, s4};
  for (byte i = 0; i < 4; i++) {
    digitalWrite(ledPin[i], states[i]);
  }
}

void playPush(byte pin, bool state) {
  byte currentVelocity = pushVelocity[pin];
  byte currentNote = pushNote[pin];
  if (!pushSettingsLocked[pin]) {
    currentVelocity = globalVelocity;
    currentNote += globalNoteOffset;
  }
  sendNote(currentNote, currentVelocity, state);
}

byte getMidiValueFromFader(byte selected) {
  if (faderVal[selected] >= MAX_FADER_VALUE) {
    faderVal[selected] = 1024;
  }
  if (faderVal[selected] <= MIN_FADER_VALUE) {
    faderVal[selected] = 0;
  }
  return ((uint32_t) faderVal[selected] * 127) / 1024;
}

float getPitchModulationFromfader(byte selected) {
  if (faderVal[selected] > 512) {
    return (float) (faderVal[selected] - 512) / 512;
  }
  else {
    return (float) faderVal[selected] / 512 * -1;
  }
}

int getMidiValueFromEncoder(byte currentMidiValue, int position, int previousPosition) {
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

char* getNoteFromMidiValue(byte midiValue) {
  return midiNote[midiValue];
}

int readFader(byte selected) {
  faderVal[selected] = analogRead(faderPin[selected]);
  if (
    faderVal[selected] > faderPos[selected] + FADER_THRESHOLD ||
    faderVal[selected] < faderPos[selected] - FADER_THRESHOLD
  ) {
    if (faderVal[selected] >= MAX_FADER_VALUE) {
      return 1024;
    }
    else if (faderVal[selected] <= MIN_FADER_VALUE) {
      return 0;
    }
    return faderVal[selected];
  }
  else {
    return faderPos[selected];
  }
}

int readEncoder(byte e) {
  int position = encoder[e].read();
  if (position != 0 && encoderPos[e] != position) {
    return position;
  }
  else {
    return encoderPos[e];
  }
}

void updateVelocity(byte globalMidiValue, byte localMidiValue) {
  if (selectedPushPin != -1 && pushSettingsLocked[selectedPushPin]) {
    pushVelocity[selectedPushPin] = localMidiValue;
    display.clear();
    display.print('v');
    displayPrint(localMidiValue, false, false);
  }
  else {
    globalVelocity = globalMidiValue;
    pushVelocity[selectedPushPin] = globalVelocity;
    display.clear();
    display.print('v');
    displayPrint(globalVelocity, false, false);
  }
}

void updateNotes(int globalMidiOffset, int localMidiOffset) {
  if (selectedPushPin != -1 && pushSettingsLocked[selectedPushPin]) {
    pushNote[selectedPushPin] = localMidiOffset;
    if (pushNote[selectedPushPin] < 0) { pushNote[selectedPushPin] = 0; }
    char* note = getNoteFromMidiValue(pushNote[selectedPushPin]);
    displayPrint(note, false, true);
  }
  else {
    globalNoteOffset = globalMidiOffset;
    if (60+globalNoteOffset < 0) { globalNoteOffset = -60; }
    if (60+globalNoteOffset > 127) { globalNoteOffset = 67; }
    char* note = getNoteFromMidiValue(60+globalNoteOffset);
    displayPrint(note, false, true);
  }
}

void moveOctave(bool up) {
  if (up && octave < 6) {
    octave++;
  }
  else if (up && octave > 6) {
    octave = 6;
  }
  else if (octave <= -4) {
    octave = -4;
  }
  else {
    octave--;
  }
  display.clear();
  if (octave >= 0) {
    display.print(String(octave) + "oct");
  }
  else {
    display.print(String(octave) + "oc");
  }
  display.setColonOn(false);
  globalNoteOffset = octave * 12;
}

void updatePadsLock(bool lock) {

  for (byte p = 0; p < NB_PUSH; p++) {

    if (isPushed[p] == PUSHED) {
      display.clear();
      if (lock == false) {
        pushSettingsLocked[p] = true;
        display.print("PAd" + String(p+1));
      }
      else {
        pushSettingsLocked[p] = false;
        display.print("GL0b");
      }
      display.setColonOn(false);
    }
  }
}

void updatePadsRepeatLockUnlock(bool isLocked) {

  for (byte p = 0; p < NB_PUSH; p++) {
    if (isPushed[p] == RELEASED) {
      continue;
    }
    if (isLocked == false) {
      displayPrintString("ULoK");
      isPushed[p] = RELEASED;
    }
    else {
      displayPrintString("Lock");
    }
    repeatIsLocked[p] = isLocked;
  }
}

void updatePads() {

  for (byte p = 0; p < NB_PUSH; p++) {

    mux.channel(pushPin[p]);
    byte sensorVal = digitalRead(MUXSIG);

    if (sensorVal == PUSHED && isPushed[p] == RELEASED) {

      isPushed[p] = PUSHED;
      selectedPushPin = p;

      if (!checkMode(CC_MASK) || (rightPush == RELEASED && leftPush == RELEASED)) {
        pushedTime[p] = micros();
        playPush(p, 1);
        if (leftPush == RELEASED && rightPush == RELEASED) {
          display.clear();
          display.setColonOn(false);
        }
      }
    }
    else if (sensorVal == RELEASED && isPushed[p] == PUSHED) {
      isPushed[p] = RELEASED;
      playPush(p, 0);
    }
  }
}

void playNotesRepeat() {
  for (byte pin = 0; pin < NB_PUSH; pin++) {
    if (
      (isPushed[pin] == RELEASED) ||
      (!checkMode(REPEAT_MASK) && repeatIsLocked[pin] == false)
    ) {
      pushedTime[pin] = getNextRepeatMicros(pin);
      pushElapsedRepeats[pin] = 0;
    }
    unsigned long nextCap = getNextRepeatMicros(pin);
    if (nextCap != 0 && micros() > nextCap) {
      if (
        (checkMode(REPEAT_MASK) && isPushed[pin] == PUSHED) ||
        repeatIsLocked[pin] == true
      ) {
        pushElapsedRepeats[pin]++;
        playPush(pin, true);
      }
    }
  }
}

float getRepeatSpeed(byte pin) {
  byte currentRepeatSpeedDivisor = pushSettingsLocked[pin] ? pushRepeatSpeed[pin][1] : repeatSpeedDivisor;
  return (float) 4 * ((float) 1 / (float) currentRepeatSpeedDivisor);
} 

void updateLedsTempo() {

  unsigned long nextNoteMicros = getNextNoteMicros();
  unsigned long loopTime = micros();

  // @todo flash oxxx then oooo if no SPP found.
  if (loopTime > (nextNoteMicros - getOneNoteFractionMicros(0.25))) {
    writeLeds(HIGH, HIGH, HIGH, HIGH);
  }
  else if (loopTime > (nextNoteMicros - getOneNoteFractionMicros(0.5))) {
    writeLeds(HIGH, HIGH, HIGH, LOW);
  }
  else if (loopTime > (nextNoteMicros - getOneNoteFractionMicros(0.75))) {
    writeLeds(HIGH, HIGH, LOW, LOW);
  }
  else if (loopTime > (nextNoteMicros - oneNoteTime)) {
    writeLeds(HIGH, LOW, LOW, LOW);
  }

}

unsigned long getOneNoteFractionMicros(float fraction) {
  return oneNoteTime * fraction;
}

unsigned long getNextNoteMicros() {
  return startTime + ((nbElapsedNotes + 1) * oneNoteTime);
}

unsigned long getNextRepeatMicros(int pin) {
  float pinRepeatSpeed = getRepeatSpeed(pin);
  unsigned long pushDuration = (float) getBeatMicros(1) * pinRepeatSpeed;
  unsigned long nextTriggerTime = pushedTime[pin] + (pushElapsedRepeats[pin] * pushDuration);
  return nextTriggerTime;
  // @todo option sans quantize pour remplacer startTime par push repeat time.
  // return startTime - 5000 + (pushElapsedRepeats[pin] * oneNoteFractionMicros) + oneNoteFractionMicros;
}

unsigned long getNoteMicros() {
  return getBeatMicros(4);
}

unsigned long getBeatMicros(int nbBeats) {
  return  (1000000 / ((float) bpm / 60)) * (float) nbBeats;
}

void updateBpm() {

  // Quarter note time:
  if (midiCLockTick % 24 == 0) {

    // BPM:
    unsigned long bpmTime = micros();
    quarterNoteTime = bpmTime - quarterNoteTime;
    unsigned long newBpm = 60000000/quarterNoteTime;
    if (bpm != newBpm && (bpm > newBpm + 1 || bpm < newBpm + 1)) {
      bpm = newBpm;
      // displayPrintInt(bpm);
      oneNoteTime = getNoteMicros();
      startTime = startTime * (bpm / newBpm);
      // @todo send stop play.
    }
    quarterNoteTime = bpmTime;
  }
}

void MidiSync() {
  updateBpm();
  midiCLockTick++;
}

void readSwitches() {

  currentPlayMode = B00000000;
  for (byte position = 0; position < 4; position++) {
    byte sw = switches[position][0];
    byte mask = switches[position][1];
    mux.channel(sw);
    if (digitalRead(MUXSIG) == true) {
      currentPlayMode |= mask;
    }
  }

  // @todo replace P1SW & P2SW with two new dedicated push buttons.
  mux.channel(P1SW);
  leftPush = digitalRead(MUXSIG);
  mux.channel(P2SW);
  rightPush = digitalRead(MUXSIG);
}

void updateMidiControlFromEncoder(byte selected) {
  if (encoderVal[selected] != encoderPos[selected]) {
    midiCC[selected] = getMidiValueFromEncoder(midiCC[selected], encoderVal[selected], encoderPos[selected]);
    encoderPos[selected] = encoderVal[selected];
    display.clear();
    display.print('C');
    displayPrint(midiCC[selected], false, false);
  }
}

void updateMidiCCValueFromEncoder(byte selected) {
  if (encoderVal[selected] == encoderPos[selected]) {
    return;
  }
  midiCCValue[selected] = getMidiValueFromEncoder(
    midiCCValue[selected],
    encoderVal[selected],
    encoderPos[selected]
  );
  MIDI.sendControlChange(midiCC[selected], midiCCValue[selected], midiChannel);
  encoderPos[selected] = encoderVal[selected];
  display.clear();
  display.print('c');
  displayPrint(midiCCValue[selected], false, false);
}

void updateCCValueFromFader(byte selected) {
  if (faderVal[selected] == faderPos[selected]) {
    return;
  }
  faderPos[selected] = faderVal[selected];
  midiCCValue[selected] = getMidiValueFromFader(selected);
  MIDI.sendControlChange(midiCC[selected], midiCCValue[selected], midiChannel);
  display.clear();
  display.print('c');
  displayPrint(midiCCValue[selected], false, false);
}

void updateVelocityFromFader(byte selected) {
  if (faderVal[selected] == faderPos[selected]) {
    return;
  }
  faderPos[selected] = faderVal[selected];
  uint16_t newMidiValue = getMidiValueFromFader(selected);
  updateVelocity(newMidiValue, newMidiValue);
}

void updateNoteRepeatSpeedFromEncoder(byte selected) {

  bool changed = false;
  byte tmpRepeatSpeedDivisor = repeatSpeedDivisor;
  if (pushSettingsLocked[selectedPushPin]) {
    tmpRepeatSpeedDivisor = pushRepeatSpeed[selectedPushPin][1];
  }

  if (encoderVal[selected] != encoderPos[selected]) {
    tmpRepeatSpeedDivisor = tmpRepeatSpeedDivisor + encoderVal[selected] - encoderPos[selected];
    if (tmpRepeatSpeedDivisor > 64) {
      tmpRepeatSpeedDivisor = 64;
    }
    if (tmpRepeatSpeedDivisor < 1) { tmpRepeatSpeedDivisor = 1; }
    encoderPos[selected] = encoderVal[selected];
    changed = true;
  }
  if (changed) {
    repeatSpeedDivisor = tmpRepeatSpeedDivisor;
    pushRepeatSpeed[selectedPushPin][1] = repeatSpeedDivisor;
    for (byte pad = 0; pad < NB_PUSH; pad++) {
      pushElapsedRepeats[pad] = 0;
      pushedTime[pad] = micros();
    }
    display.clear();
    display.print(" 1" + String(repeatSpeedDivisor));
    display.setColonOn(true);
  }
}

// @todo debug why slow.
void updateUltrasonicCC(byte selected) {
  if (encoderVal[selected] != encoderPos[selected]) {
    ultrasonicCC = getMidiValueFromEncoder(ultrasonicCC, encoderVal[selected], encoderPos[selected]);
    display.clear();
    display.print('C');
    displayPrint(ultrasonicCC, false, false);
    encoderPos[selected] = encoderVal[selected];
  }
}

void updateUltrasonicDistance(byte selected) {
  if (encoderVal[selected] != encoderPos[selected]) {
    maxUltrasonicDistanceCm = maxUltrasonicDistanceCm + (encoderVal[selected] - encoderPos[selected]);
    display.clear();
    display.print('d');
    displayPrint(maxUltrasonicDistanceCm, false, false);
    encoderPos[selected] = encoderVal[selected];
  }
}

void trackUltrasonicChanges() {
  byte distance = distanceSensor.measureDistanceCm();
  distance = distance > maxUltrasonicDistanceCm ? maxUltrasonicDistanceCm : distance;
  distance = distance < MIN_ULTRASONIC_DISTANCE_CM ? MIN_ULTRASONIC_DISTANCE_CM : distance;
  float distancePercentage = 1 - ((float) (distance - MIN_ULTRASONIC_DISTANCE_CM) / (float) (maxUltrasonicDistanceCm - MIN_ULTRASONIC_DISTANCE_CM));
  byte ultrasonicControlValue = distancePercentage * 127;
  MIDI.sendControlChange(ultrasonicCC, ultrasonicControlValue, midiChannel);
}

void selectCCPreset(byte selected) {

  for (byte p = 0; p < NB_PUSH; p++) {

    if (isPushed[p] == RELEASED || repeatIsLocked[p] == true) {
      continue;
    }

    midiCC[selected] = midiCCPresets[p];
    // Problème d'affichage
    display.clear();
    display.print('P');
    displayPrint(midiCCPresets[p], false, false);
  }
}

void displayPrint(const char string[], bool semicolon, bool clear) {
  prepareDisplay(semicolon, clear);
  display.print(string);
}

void displayPrint(int string, bool semicolon, bool clear) {
  prepareDisplay(semicolon, clear);
  display.print(string);
}

void displayPrintString(String s) {
  display.setColonOn(false);
  display.clear();
  display.print(s);
}

void prepareDisplay(bool semicolon, bool clear) {
  semicolon ? display.setColonOn(true) : display.setColonOn(false);
  if (clear) {
    display.clear();
  }
}

bool checkMode(byte mask) {
  return ((currentPlayMode & mask) == mask);
}

void sendNote(byte note, byte velocity, bool on) {
  for (
    byte offset = 0;
    offset < (MAX_NOTES - 1);
    offset++
  ) {
    if (
      (note + chords[selectedChord][offset]) >= 128 ||
      chords[selectedChord][offset] == UNASSIGNED
    ) {
      return;
    }
    on ?
      MIDI.sendNoteOn(note + chords[selectedChord][offset], velocity, midiChannel) :
      MIDI.sendNoteOff(note + chords[selectedChord][offset], velocity, midiChannel);
  }

  if (on) {
    digitalWrite(LED_BUILTIN, HIGH);
    digitalWrite(MAGNET, HIGH);
  }
  else {
    digitalWrite(LED_BUILTIN, LOW);
    digitalWrite(MAGNET, LOW);
  }
}
