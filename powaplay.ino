/**
 * MIDI P0Ah PLAy
 */

#include "MIDI.h"
#include "SevenSegmentTM1637.h"
#include "SevenSegmentExtended.h"
#include "SevenSegmentFun.h"
#include <Encoder.h>
#include <CD74HC4067.h>
#include <HCSR04.h>

const unsigned long BAUD_RATE PROGMEM = 38400;

// Misc

const byte UNASSIGNED PROGMEM = 255;

// Push buttons:

const byte PUSHED PROGMEM = LOW;
const byte RELEASED PROGMEM = HIGH;
const byte NB_PUSH PROGMEM = 8;

// Encoders:

const byte NB_ENCODERS PROGMEM = 2;

const byte P1CLK PROGMEM = 2;
const byte P1DT PROGMEM = 4;
const byte P1SW PROGMEM = 15;
const byte P2CLK PROGMEM = 3;
const byte P2DT PROGMEM = 5;
const byte P2SW PROGMEM = 14;

Encoder P1(P1CLK, P1DT);
Encoder P2(P2CLK, P2DT);

Encoder* encoder[NB_ENCODERS] = {&P1, &P2};
// Old state:
int encoderPos[NB_ENCODERS] = {0, 0};
// New state:
int encoderVal[NB_ENCODERS] = {0, 0};

const byte ENCODER_STEP PROGMEM = 4;

// Multiplexer

const byte MUXSIG PROGMEM = A0;
CD74HC4067 mux(A1, A2, A3, A4);

// MIDI

struct HairlessMidiSettings : public midi::DefaultSettings
{
   static const bool UseRunningStatus = false;
   static const long BaudRate = BAUD_RATE;
};

MIDI_CREATE_CUSTOM_INSTANCE(HardwareSerial, Serial, MIDI, HairlessMidiSettings);

byte midiCC[2] = {20, 21};
byte midiCCValue[2] = {63, 63};
byte lastSentCCNumber[2] = {255, 255};
byte lastSentCCValue[2] = {255, 255};
// https://professionalcomposers.com/midi-cc-list/
// 5, 7, 10, 71, 72, 73, 74, 80, 81, 84, 91, 92, 93, 94, 95 - 98-101.
const byte midiCCPresets[NB_PUSH] PROGMEM = {91, 92, 93, 94, 95, 98, 99, 100};
byte midiChannel = 2;
byte programChange = 0;
byte globalVelocity = 127;
int globalNoteOffset = 0;
const byte ticksPerNote PROGMEM = 96;
unsigned long startTime = 0;
long nbElapsedNotes = 0;
const int MIDI_START_OFFSET PROGMEM = 0;
unsigned long lastClockPulse = 0;
unsigned long lastNoteRepeat = 0;

// LCD

const byte LCD_CLK PROGMEM = 12;
const byte LCD_DIO PROGMEM = 11;

SevenSegmentFun display(LCD_CLK, LCD_DIO);

// Faders:

const byte F1 PROGMEM = A6;
const byte F2 PROGMEM = A7;

const byte faderPin[2] PROGMEM = {F1, F2};
const byte NB_FADERS PROGMEM = 2;
uint16_t faderPos[NB_FADERS] = {0, 0};
uint16_t faderVal[NB_FADERS] = {0, 0};

const uint16_t MAX_FADER_VALUE = 970;
const uint16_t MIN_FADER_VALUE = 50;
const uint16_t FADER_THRESHOLD = 30;

// Leds:

const byte L1 PROGMEM = 10;
const byte L2 PROGMEM = 9;
const byte L3 PROGMEM = 8;
const byte L4 PROGMEM = 7;

// Ultrasonic

const byte triggerPin PROGMEM = 13;
const byte echoPin PROGMEM = 6;

UltraSonicDistanceSensor distanceSensor(triggerPin, echoPin);

byte MIN_ULTRASONIC_DISTANCE_CM = 2;
int maxUltrasonicDistanceCm = 10;
byte ultrasonicCC = 100;
byte lastUltrasonicControlValue = 255;
const int MAX_ULTRASONIC_DISTANCE_CAP_CM = 35;
const float ULTRASONIC_SMOOTHING_ALPHA = 0.35;
const byte ULTRASONIC_CC_DEADBAND = 2;
const unsigned long ULTRASONIC_MIN_UPDATE_INTERVAL_US = 15000;
int ultrasonicMedianBuffer[3] = {0, 0, 0};
byte ultrasonicMedianCount = 0;
byte ultrasonicMedianIndex = 0;
int lastValidUltrasonicDistanceCm = -1;
float smoothedUltrasonicDistanceCm = -1.0;
unsigned long lastUltrasonicUpdateMicros = 0;

// Magnet

const byte MAGNET PROGMEM = A5;

// Switches

const byte SW_CC PROGMEM = 0;
const byte SW_REPEAT PROGMEM = 1;
const byte SW_ULTRASONIC PROGMEM = 10;
const byte SW_PLAY PROGMEM = 11;

const byte INIT_MASK PROGMEM =       B00000010;
const byte CC_MASK PROGMEM =         B00000100;
const byte REPEAT_MASK PROGMEM =     B00001000;
const byte ULTRASONIC_MASK PROGMEM = B00010000;

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

const byte pushPin[NB_PUSH] = {4, 3, 2, 5, 6, 7, 8, 9};
byte pushNote[NB_PUSH];
byte pushVelocity[NB_PUSH] = {100, 100, 100, 100, 100, 100, 100, 100};
bool pushSettingsLocked[NB_PUSH] = {false, false, false, false, false, false, false, false};
byte pushRepeatSpeed[NB_PUSH][2] = {{1,4}, {1,4}, {1,4}, {1,4}, {1,4}, {1,4}, {1,4}, {1,4}};
// Track last NoteOn per pad so NoteOff always matches, even if octave/scale changes while held.
byte padActiveNote[NB_PUSH] = {0,0,0,0,0,0,0,0};
bool padNoteIsOn[NB_PUSH] = {false,false,false,false,false,false,false,false};
// For repeat mode: schedule a NoteOff some microseconds after NoteOn to create a real gate.
unsigned long padScheduledOffMicros[NB_PUSH] = {0,0,0,0,0,0,0,0};
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

// Switch edge tracking (for SW_PLAY panic on press)
bool prevInitMode = false;

// Chords
// Including NOTE (no chord).
const byte NB_CHORDS PROGMEM = 10;
const byte MAX_NOTES PROGMEM = 8;
byte selectedChord = 0;
const char* CHORD_NAMES[NB_CHORDS] = {
  "NOTE", // Single Note
  "MAJ ", // Major
  "MIN ", // Minor
  "AUG ", // Augmented
  "dIM ", // Diminished
  "SUS2", // Suspended 2
  "SUS4", // Suspended 4
  " 7th", // Dominant 7th
  "MAJ7", // Major 7th
  "MIN7"  // Minor 7th
};

// Scales
const byte NB_SCALES PROGMEM = 5;
byte selectedScale = 0;

void reinit() {
  midiCC[0] = 20;
  midiCC[1] = 21;
  midiCCValue[0] = 63;
  midiCCValue[1] = 63;
  lastSentCCNumber[0] = 255;
  lastSentCCNumber[1] = 255;
  lastSentCCValue[0] = 255;
  lastSentCCValue[1] = 255;
  midiChannel = 2;
  programChange = 0;
  globalVelocity = 127;
  globalNoteOffset = 0;
  ultrasonicCC = 100;
  maxUltrasonicDistanceCm = 10;
  lastUltrasonicControlValue = 255;
  ultrasonicMedianBuffer[0] = 0;
  ultrasonicMedianBuffer[1] = 0;
  ultrasonicMedianBuffer[2] = 0;
  ultrasonicMedianCount = 0;
  ultrasonicMedianIndex = 0;
  lastValidUltrasonicDistanceCm = -1;
  smoothedUltrasonicDistanceCm = -1.0;
  lastUltrasonicUpdateMicros = 0;
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

  for (byte i = 0; i < 8; i++) {
    padNoteIsOn[i] = false;
    padActiveNote[i] = 0;
  }
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

void loop() {

  unsigned long loopTime = micros();

  updateMidiSerial();
  processScheduledNoteOffs();
  if (playFlag == false) {
    playNotesRepeat();
  }

  readSwitches();
  // Panic: when SW_PLAY is pressed (mapped to INIT_MASK), send All Notes Off.
  // We detect the rising edge of INIT_MASK.
  bool initMode = checkMode(INIT_MASK);
  if (initMode && !prevInitMode) {
    panicAllNotesOff();
  }
  prevInitMode = initMode;
  updatePads();

  // Update LEDs AFTER pad/switch scan so they always reflect current pad state.
  if (playFlag) {
    // While synced: pads override tempo while held.
    if (aPadIsPushed()) {
      updateLedsPads();
    }
    else {
      updateLedsTempo();
    }
  }
  else {
    updateLedsPads();
  }

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
      updateMidiControlFromEncoder(1);
      selectCCPreset(1);
    }
    if (leftPush == PUSHED) {
      updateMidiControlFromEncoder(0);
      selectCCPreset(0);
    }
  }
  else {
    // chords and scales
    if (rightPush == PUSHED) {
      scaleSelect(0);
      updatePadsLock(true);
    }
    if (leftPush == PUSHED) {
      chordSelect(1);
      updatePadsLock(false);
    }
  }

  if (checkMode(ULTRASONIC_MASK)) {
    trackUltrasonicChanges();
  }
}

// Pads -> LEDs mapping (columns):
// LED1: pads 1 & 5 (index 0 & 4)
// LED2: pads 2 & 6 (index 1 & 5)
// LED3: pads 3 & 7 (index 2 & 6)
// LED4: pads 4 & 8 (index 3 & 7)
void updateLedsPads() {
  // Light an LED whenever *any* pad in its column is currently pushed.
  // (Does not track history; always reflects current pad state.)
  byte s1 = (isPushed[0] == PUSHED || isPushed[4] == PUSHED) ? HIGH : LOW;
  byte s2 = (isPushed[1] == PUSHED || isPushed[5] == PUSHED) ? HIGH : LOW;
  byte s3 = (isPushed[2] == PUSHED || isPushed[6] == PUSHED) ? HIGH : LOW;
  byte s4 = (isPushed[3] == PUSHED || isPushed[7] == PUSHED) ? HIGH : LOW;
  writeLeds(s1, s2, s3, s4);
}

void updateChannelFromEncoder(byte selected) {
  if (encoderVal[selected] != encoderPos[selected]) {
    int delta = encoderVal[selected] - encoderPos[selected];
    int wrappedChannel = ((int)midiChannel - 1 + delta) % 16;
    if (wrappedChannel < 0) {
      wrappedChannel += 16;
    }
    midiChannel = wrappedChannel + 1;
    display.clear();
    display.setColonOn(false);
    display.print("CH" + String(midiChannel));
    encoderPos[selected] = encoderVal[selected];
  }
}

void updateBaseNoteFromEncoder(byte selected) {
  if (encoderVal[selected] != encoderPos[selected]) {
    byte localPushNote = selectedPushPin != -1 ? pushNote[selectedPushPin] : (60 + globalNoteOffset);
    updateNotes(
      (getMidiValueFromEncoder(60+globalNoteOffset, encoderVal[selected], encoderPos[selected]) - 60),
      getMidiValueFromEncoder(localPushNote, encoderVal[selected], encoderPos[selected])
    );
  }
  encoderPos[selected] = encoderVal[selected];
}

void chordSelect(byte selected) {

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
  display.print(CHORD_NAMES[selectedChord]);
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
    char label[8];
    snprintf(label, sizeof(label), "%doct", octave);
    display.print(label);
  }
  else {
    char label[8];
    snprintf(label, sizeof(label), "%doc", octave);
    display.print(label);
  }
  globalNoteOffset = octave * 12;
  faderPos[selected] = faderVal[selected];

  // Live transposition: if pads are currently held, retrigger them
  // so the pitch follows the octave change.
  retriggerHeldPads();
}

void updateVelocityFromEncoder(byte selected) { 
  if (encoderVal[selected] == encoderPos[selected]) {
    return;
  }
  byte localPushVelocity = selectedPushPin != -1 ? pushVelocity[selectedPushPin] : globalVelocity;
  updateVelocity(
    getMidiValueFromEncoder(globalVelocity, encoderVal[selected], encoderPos[selected]),
    getMidiValueFromEncoder(localPushVelocity, encoderVal[selected], encoderPos[selected])
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
    // Re-align beat phase on transport start.
    midiCLockTick = 0;
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

void scaleSelect(byte selected) {

  const char* SCALES_NAMES[NB_SCALES] = {
    "SEMI",
    "MAJ",
    "MIN_",
    "BLUE",
    "BLU_"
  };

  encoderVal[selected] = readEncoder(selected);
  if (encoderVal[selected] == encoderPos[selected]) {
    return;
  }

  // @todo mutualiser la logique de sélection avec chords:
  // Scale:
  if (encoderVal[selected] > encoderPos[selected]) {
    selectedScale = selectedScale < (NB_SCALES-1) ? selectedScale+1 : selectedScale;
  }
  else if (selectedScale > 0) {
    selectedScale = selectedScale > 0 ? selectedScale - 1 : 0;
  }
  display.clear();
  display.print(SCALES_NAMES[selectedScale]);
  display.setColonOn(false);
  encoderPos[selected] = encoderVal[selected];
}

void playPush(byte pin, bool state) {

  // @todo complete & fix
  byte SCALES[NB_SCALES][MAX_NOTES] = {
    {0, 0, 0, 0, 0, 0, 0, 0},
    {0, 1, 2, 2, 3, 4, 5, 5}, // Major
    {0, 0, 3, 2, 2, 1, 2, 2}, // Minor
    {0, 1, 4, 1, 2, UNASSIGNED, UNASSIGNED, UNASSIGNED}, // Blues
    {1, 1, 3, 2, 1, UNASSIGNED, UNASSIGNED, UNASSIGNED}, // Blues minor
  };

  byte currentVelocity = pushVelocity[pin];
  byte currentNote = pushNote[pin];
  if (!pushSettingsLocked[pin]) {
    currentVelocity = globalVelocity;
    currentNote += globalNoteOffset;
  }

  if (SCALES[selectedScale][pin] == UNASSIGNED) {
    return;
  }

  currentNote += SCALES[selectedScale][pin];

  if (state) {
    // Ensure we don't leave a previous note on (important for repeat & live transposition)
    if (padNoteIsOn[pin]) {
      sendNote(padActiveNote[pin], 0, false);
      padNoteIsOn[pin] = false;
    }

    padActiveNote[pin] = currentNote;
    padNoteIsOn[pin] = true;
    sendNote(currentNote, currentVelocity, true);
  }
  else {
    // Prefer turning off what we actually turned on.
    if (padNoteIsOn[pin]) {
      sendNote(padActiveNote[pin], 0, false);
      padNoteIsOn[pin] = false;
    }
  }
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

const char* getNoteFromMidiValue(byte midiValue) {
  // Lightweight formatter to avoid allocating 128 Strings on each call.
  static char note[5];
  const char* names[12] = {
    "C", "d", "D", "E", "E", "F", "F", "G", "A", "A", "b", "b"
  };
  const char accidental[12] = {
    ' ', 'b', ' ', 'b', ' ', ' ', 'b', ' ', 'b', ' ', 'b', ' '
  };

  byte n = midiValue % 12;
  byte oct = midiValue / 12;
  // Keep same compact style as existing display strings (4 chars max)
  // e.g. " C4 ", "Eb4 ", "A10 " -> compressed for 4-char display.
  if (oct < 10) {
    snprintf(note, sizeof(note), "%1s%c%1u", names[n], accidental[n], oct);
  }
  else {
    // Two-digit octave fallback
    snprintf(note, sizeof(note), "%1s%1u%1u", names[n], oct / 10, oct % 10);
  }
  return note;
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
  int position = encoder[e]->read();
  if (encoderPos[e] != position) {
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
    if (selectedPushPin != -1) {
      pushVelocity[selectedPushPin] = globalVelocity;
    }
    display.clear();
    display.print('v');
    displayPrint(globalVelocity, false, false);
  }
}

void updateNotes(int globalMidiOffset, int localMidiOffset) {
  if (selectedPushPin != -1 && pushSettingsLocked[selectedPushPin]) {
    pushNote[selectedPushPin] = localMidiOffset;
    if (pushNote[selectedPushPin] < 0) { pushNote[selectedPushPin] = 0; }
    const char* note = getNoteFromMidiValue(pushNote[selectedPushPin]);
    displayPrintString(note);
  }
  else {
    globalNoteOffset = globalMidiOffset;
    if (60+globalNoteOffset < 0) { globalNoteOffset = -60; }
    if (60+globalNoteOffset > 127) { globalNoteOffset = 67; }
    const char* note = getNoteFromMidiValue(60+globalNoteOffset);
    displayPrintString(note);
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
    char label[8];
    snprintf(label, sizeof(label), "%doct", octave);
    display.print(label);
  }
  else {
    char label[8];
    snprintf(label, sizeof(label), "%doc", octave);
    display.print(label);
  }
  display.setColonOn(false);
  globalNoteOffset = octave * 12;

  // Live transposition for encoder octave changes too.
  retriggerHeldPads();
}

void retriggerHeldPads() {
  // For each currently-held pad: NoteOff the previously sounding note, then NoteOn the new one.
  // This avoids stuck notes and gives live transposition.
  for (byte p = 0; p < NB_PUSH; p++) {
    if (isPushed[p] != PUSHED) {
      continue;
    }
    if (padNoteIsOn[p]) {
      // Use velocity 0 for safety / compatibility.
      sendNote(padActiveNote[p], 0, false);
      padNoteIsOn[p] = false;
    }
    // Recompute and trigger the new note.
    playPush(p, true);
  }
}

unsigned long getRepeatGateMicros(byte pin) {
  // Gate = 50% of repeat period, clamped to avoid being too short (inaudible)
  // or too long (overlapping notes).
  float pinRepeatSpeed = getRepeatSpeed(pin);
  unsigned long repeatPeriod = (float) getBeatMicros(1) * pinRepeatSpeed;
  unsigned long gate = repeatPeriod / 2;
  if (gate < 8000) {
    gate = 8000;
  }
  if (gate > 80000) {
    gate = 80000;
  }
  return gate;
}

void processScheduledNoteOffs() {
  unsigned long now = micros();
  for (byte p = 0; p < NB_PUSH; p++) {
    if (padScheduledOffMicros[p] == 0) {
      continue;
    }
    // Handle micros() overflow safely
    if ((long)(now - padScheduledOffMicros[p]) >= 0) {
      if (padNoteIsOn[p]) {
        sendNote(padActiveNote[p], 0, false);
        padNoteIsOn[p] = false;
      }
      padScheduledOffMicros[p] = 0;
    }
  }
}

void panicAllNotesOff() {
  // Send All Notes Off (CC123) + All Sound Off (CC120), then clear local tracking.
  // This is a safety net if something ever gets stuck.
  MIDI.sendControlChange(123, 0, midiChannel);
  MIDI.sendControlChange(120, 0, midiChannel);

  for (byte p = 0; p < NB_PUSH; p++) {
    if (padNoteIsOn[p]) {
      sendNote(padActiveNote[p], 0, false);
    }
    padNoteIsOn[p] = false;
    padActiveNote[p] = 0;
    padScheduledOffMicros[p] = 0;
    isPushed[p] = RELEASED;
    repeatIsLocked[p] = false;
  }
}

void updatePadsLock(bool lock) {

  for (byte p = 0; p < NB_PUSH; p++) {

    if (isPushed[p] == PUSHED) {
      display.clear();
      if (lock == false) {
        pushSettingsLocked[p] = true;
        char label[6];
        snprintf(label, sizeof(label), "PAd%u", p + 1);
        display.print(label);
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
      // Stop any currently sounding note for this pad.
      if (padNoteIsOn[p]) {
        sendNote(padActiveNote[p], 0, false);
        padNoteIsOn[p] = false;
      }
      padScheduledOffMicros[p] = 0;
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
      // Ensure NoteOff matches what we actually turned on.
      if (padNoteIsOn[p]) {
        sendNote(padActiveNote[p], 0, false);
        padNoteIsOn[p] = false;
      }
      padScheduledOffMicros[p] = 0;
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
        // Create a real gate time for repeat notes.
        padScheduledOffMicros[pin] = micros() + getRepeatGateMicros(pin);
      }
    }
  }
}

float getRepeatSpeed(byte pin) {
  byte currentRepeatSpeedDivisor = pushSettingsLocked[pin] ? pushRepeatSpeed[pin][1] : repeatSpeedDivisor;
  return (float) 4 * ((float) 1 / (float) currentRepeatSpeedDivisor);
} 

void updateLedsTempo() {
  // Tick-based LED phase (stable):
  // MIDI clock = 24 ticks/quarter => 96 ticks / bar (4/4)
  // Show one LED per quarter of the bar.
  byte step = midiCLockTick % 96;

  if (step < 24) {
    writeLeds(HIGH, LOW, LOW, LOW);
  }
  else if (step < 48) {
    writeLeds(LOW, HIGH, LOW, LOW);
  }
  else if (step < 72) {
    writeLeds(LOW, LOW, HIGH, LOW);
  }
  else {
    writeLeds(LOW, LOW, LOW, HIGH);
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

bool sendControlChangeIfChanged(byte lane, byte ccNumber, byte ccValue) {
  if (lastSentCCNumber[lane] == ccNumber && lastSentCCValue[lane] == ccValue) {
    return false;
  }
  MIDI.sendControlChange(ccNumber, ccValue, midiChannel);
  lastSentCCNumber[lane] = ccNumber;
  lastSentCCValue[lane] = ccValue;
  return true;
}

bool shouldSuppressLiveCCDisplay() {
  return (leftPush == PUSHED || rightPush == PUSHED);
}

void updateMidiCCValueFromEncoder(byte selected) {
  if (encoderVal[selected] == encoderPos[selected]) {
    return;
  }
  byte newCCValue = getMidiValueFromEncoder(
    midiCCValue[selected],
    encoderVal[selected],
    encoderPos[selected]
  );
  encoderPos[selected] = encoderVal[selected];
  if (!sendControlChangeIfChanged(selected, midiCC[selected], newCCValue)) {
    return;
  }
  midiCCValue[selected] = newCCValue;
  if (shouldSuppressLiveCCDisplay()) {
    return;
  }
  display.clear();
  display.print('c');
  displayPrint(midiCCValue[selected], false, false);
}

void updateCCValueFromFader(byte selected) {
  if (faderVal[selected] == faderPos[selected]) {
    return;
  }
  faderPos[selected] = faderVal[selected];
  byte newCCValue = getMidiValueFromFader(selected);
  if (!sendControlChangeIfChanged(selected, midiCC[selected], newCCValue)) {
    return;
  }
  midiCCValue[selected] = newCCValue;
  if (shouldSuppressLiveCCDisplay()) {
    return;
  }
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

  if (aPadIsPushed()) {
    return;
  }

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
    char label[6];
    snprintf(label, sizeof(label), " 1%u", repeatSpeedDivisor);
    display.print(label);
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

    // Allow negative distances to reverse ultrasonic CC mapping.
    if (maxUltrasonicDistanceCm > MAX_ULTRASONIC_DISTANCE_CAP_CM) {
      maxUltrasonicDistanceCm = MAX_ULTRASONIC_DISTANCE_CAP_CM;
    }
    if (maxUltrasonicDistanceCm < -MAX_ULTRASONIC_DISTANCE_CAP_CM) {
      maxUltrasonicDistanceCm = -MAX_ULTRASONIC_DISTANCE_CAP_CM;
    }

    // Keep 0 unavailable so sign always means CC direction mode.
    if (maxUltrasonicDistanceCm == 0) {
      maxUltrasonicDistanceCm = (encoderVal[selected] >= encoderPos[selected]) ? 1 : -1;
    }

    display.clear();
    display.print('d');
    displayPrint(maxUltrasonicDistanceCm, false, false);
    encoderPos[selected] = encoderVal[selected];
  }
}

int medianOf3(int a, int b, int c) {
  if (a > b) {
    int tmp = a;
    a = b;
    b = tmp;
  }
  if (b > c) {
    int tmp = b;
    b = c;
    c = tmp;
  }
  if (a > b) {
    int tmp = a;
    a = b;
    b = tmp;
  }
  return b;
}

void addUltrasonicSample(int sampleCm) {
  ultrasonicMedianBuffer[ultrasonicMedianIndex] = sampleCm;
  ultrasonicMedianIndex = (ultrasonicMedianIndex + 1) % 3;
  if (ultrasonicMedianCount < 3) {
    ultrasonicMedianCount++;
  }
}

int getUltrasonicMedianCm() {
  if (ultrasonicMedianCount == 0) {
    return 0;
  }
  if (ultrasonicMedianCount < 3) {
    long sum = 0;
    for (byte i = 0; i < ultrasonicMedianCount; i++) {
      sum += ultrasonicMedianBuffer[i];
    }
    return (int) (sum / ultrasonicMedianCount);
  }
  return medianOf3(
    ultrasonicMedianBuffer[0],
    ultrasonicMedianBuffer[1],
    ultrasonicMedianBuffer[2]
  );
}

void trackUltrasonicChanges() {
  unsigned long now = micros();
  if ((long) (now - lastUltrasonicUpdateMicros) < (long) ULTRASONIC_MIN_UPDATE_INTERVAL_US) {
    return;
  }
  lastUltrasonicUpdateMicros = now;

  int measuredDistanceCm = (int) distanceSensor.measureDistanceCm();
  if (measuredDistanceCm > 0) {
    addUltrasonicSample(measuredDistanceCm);
    int medianDistanceCm = getUltrasonicMedianCm();

    if (smoothedUltrasonicDistanceCm < 0.0) {
      smoothedUltrasonicDistanceCm = medianDistanceCm;
    }
    else {
      smoothedUltrasonicDistanceCm =
        (ULTRASONIC_SMOOTHING_ALPHA * (float) medianDistanceCm)
        + ((1.0 - ULTRASONIC_SMOOTHING_ALPHA) * smoothedUltrasonicDistanceCm);
    }

    measuredDistanceCm = (int) round(smoothedUltrasonicDistanceCm);
    lastValidUltrasonicDistanceCm = measuredDistanceCm;
  }
  else {
    if (lastValidUltrasonicDistanceCm < 0) {
      return;
    }
    measuredDistanceCm = lastValidUltrasonicDistanceCm;
  }

  int configuredDistanceCm = maxUltrasonicDistanceCm;
  int absoluteMaxDistanceCm = abs(configuredDistanceCm);

  if (absoluteMaxDistanceCm < 1) {
    absoluteMaxDistanceCm = 1;
  }
  if (absoluteMaxDistanceCm > MAX_ULTRASONIC_DISTANCE_CAP_CM) {
    absoluteMaxDistanceCm = MAX_ULTRASONIC_DISTANCE_CAP_CM;
  }
  if (absoluteMaxDistanceCm < MIN_ULTRASONIC_DISTANCE_CM) {
    absoluteMaxDistanceCm = MIN_ULTRASONIC_DISTANCE_CM;
  }

  if (measuredDistanceCm > absoluteMaxDistanceCm) {
    measuredDistanceCm = absoluteMaxDistanceCm;
  }
  if (measuredDistanceCm < MIN_ULTRASONIC_DISTANCE_CM) {
    measuredDistanceCm = MIN_ULTRASONIC_DISTANCE_CM;
  }

  // blockedPercentage: 1.0 when fully blocked (near), 0.0 when unblocked (far).
  int distanceSpanCm = absoluteMaxDistanceCm - MIN_ULTRASONIC_DISTANCE_CM;
  if (distanceSpanCm <= 0) {
    distanceSpanCm = 1;
  }
  float blockedPercentage = 1.0 - (
    (float) (measuredDistanceCm - MIN_ULTRASONIC_DISTANCE_CM)
    /
    (float) distanceSpanCm
  );

  byte ultrasonicControlValue;
  if (configuredDistanceCm < 0) {
    // Reversed mode: far -> 0, blocked -> 127
    ultrasonicControlValue = blockedPercentage * 127;
  }
  else {
    // Default mode: far -> 127, blocked -> 0
    ultrasonicControlValue = (1.0 - blockedPercentage) * 127;
  }

  if (lastUltrasonicControlValue != 255) {
    int ccDelta = (int) ultrasonicControlValue - (int) lastUltrasonicControlValue;
    if (ccDelta < 0) {
      ccDelta *= -1;
    }
    if (ccDelta < ULTRASONIC_CC_DEADBAND) {
      return;
    }
  }

  if (ultrasonicControlValue != lastUltrasonicControlValue) {
    MIDI.sendControlChange(ultrasonicCC, ultrasonicControlValue, midiChannel);
    if (!shouldSuppressLiveCCDisplay()) {
      display.clear();
      display.print('c');
      displayPrint(ultrasonicControlValue, false, false);
    }
    lastUltrasonicControlValue = ultrasonicControlValue;
  }
}

void selectCCPreset(byte selected) {

  for (byte p = 0; p < NB_PUSH; p++) {

    if (isPushed[p] == RELEASED || repeatIsLocked[p] == true) {
      continue;
    }

    byte presetCC = pgm_read_byte(&midiCCPresets[p]);
    midiCC[selected] = presetCC;
    display.clear();
    display.setColonOn(false);
    char label[5];
    snprintf(label, sizeof(label), "C%03u", presetCC);
    display.print(label);
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

void displayPrintString(const char s[]) {
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

  // @see https://spinditty.com/learning/chord-building-for-musicians
  byte CHORDS[NB_CHORDS][MAX_NOTES] = {
    {0, UNASSIGNED, UNASSIGNED, UNASSIGNED, UNASSIGNED, UNASSIGNED, UNASSIGNED, UNASSIGNED}, // NOTE
    {0, 4, 7, UNASSIGNED, UNASSIGNED, UNASSIGNED, UNASSIGNED, UNASSIGNED}, // MAJ
    {0, 3, 7, UNASSIGNED, UNASSIGNED, UNASSIGNED, UNASSIGNED, UNASSIGNED}, // MIN
    {0, 4, 8, UNASSIGNED, UNASSIGNED, UNASSIGNED, UNASSIGNED, UNASSIGNED}, // AUG
    {0, 3, 6, UNASSIGNED, UNASSIGNED, UNASSIGNED, UNASSIGNED, UNASSIGNED}, // dIM
    {0, 2, 7, UNASSIGNED, UNASSIGNED, UNASSIGNED, UNASSIGNED, UNASSIGNED}, // SUS2
    {0, 5, 7, UNASSIGNED, UNASSIGNED, UNASSIGNED, UNASSIGNED, UNASSIGNED}, // SUS4
    {0, 4, 7, 10, UNASSIGNED, UNASSIGNED, UNASSIGNED, UNASSIGNED},          // 7th
    {0, 4, 7, 11, UNASSIGNED, UNASSIGNED, UNASSIGNED, UNASSIGNED},          // MAJ7
    {0, 3, 7, 10, UNASSIGNED, UNASSIGNED, UNASSIGNED, UNASSIGNED},          // MIN7
  };

  for (
    byte offset = 0;
    offset < (MAX_NOTES - 1);
    offset++
  ) {
    byte interval = CHORDS[selectedChord][offset];
    if (interval == UNASSIGNED) {
      // End of chord definition; do not leave function so magnet state can be updated.
      break;
    }
    if ((note + interval) >= 128) {
      // Skip invalid note but keep processing and update outputs below.
      continue;
    }
    on ?
      MIDI.sendNoteOn(note + interval, velocity, midiChannel) :
      MIDI.sendNoteOff(note + interval, velocity, midiChannel);
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

bool aPadIsPushed() {
  for (byte p = 0; p < NB_PUSH; p++) {
    if (isPushed[p] == PUSHED) {
      return true;
    }
  }
  return false;
}
