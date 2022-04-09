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

// Push buttons:

const byte PUSHED = LOW;
const byte RELEASED = HIGH;
const uint8_t NB_PUSH = 8;

// Encoders:

uint8_t encoderValue = 127;

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
  " C0 ", " C#0", " d0 ", " d#0", " E0 ", " F0 ", " F#0", " G0 ", " G#0", " A0 ", " A#0", " b0 ",
  " C1 ", " C#1", " d1 ", " d#1", " E1 ", " F1 ", " F#1", " G1 ", " G#1", " A1 ", " A#1", " b1 ",
  " C2 ", " C#2", " d2 ", " d#2", " E2 ", " F2 ", " F#2", " G2 ", " G#2", " A2 ", " A#2", " b2 ",
  " C3 ", " C#3", " d3 ", " d#3", " E3 ", " F3 ", " F#3", " G3 ", " G#3", " A3 ", " A#3", " b3 ",
  " C4 ", " C#4", " d4 ", " d#4", " E4 ", " F4 ", " F#4", " G4 ", " G#4", " A4 ", " A#4", " b4 ",
  " C5 ", " CH5", " d5 ", " dH5", " E5 ", " F5 ", " FH5", " G5 ", " GH5", " A5 ", " AH5", " b5 ",
  " C6 ", " CH6", " d6 ", " dH6", " E6 ", " F6 ", " FH6", " G6 ", " GH6", " A6 ", " AH6", " b6 ",
  " C7 ", " CH7", " d7 ", " dH7", " E7 ", " F7 ", " FH7", " G7 ", " GH7", " A7 ", " AH7", " b7 ",
  " C8 ", " CH8", " d8 ", " dH8", " E8 ", " F8 ", " FH8", " G8 ", " GH8", " A8 ", " AH8", " b8 ",
  " C9 ", " CH9", " d9 ", " dH9", " E9 ", " F9 ", " FH9", " G9 ", " GH9", "A10 ", "AH10", "b10 ",
  "C10 ", "CH10", "d10 ", "dH10", "E10 ", "F10 ", "FH10", "G10 ",
};

// LCD

const uint8_t LCD_CLK = 12;
const uint8_t LCD_DIO = 11;

SevenSegmentFun display(LCD_CLK, LCD_DIO);

// Faders:

uint16_t faderValue = 0;

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

uint8_t MIN_ULTRASONIC_DISTANCE_CM = 5;
uint8_t maxUltrasonicDistanceCm = 10;
uint8_t ultrasonicCC = 100;

// Magnet

const uint8_t MAGNET = A5;

// Switches

const uint8_t SW_CC = 0;
const uint8_t SW_REPEAT = 1;
const uint8_t SW_ULTRASONIC = 10;
const uint8_t SW_PLAY = 11;

const byte STOP_MASK =       B00000001;
const byte CC_MASK =         B00000010;
const byte REPEAT_MASK =     B00000100;
const byte ULTRASONIC_MASK = B00001000;

byte currentPlayMode = B00000000;

byte switches[4][2] = {{SW_CC, CC_MASK}, {SW_PLAY, STOP_MASK}, {SW_ULTRASONIC, ULTRASONIC_MASK}, {SW_REPEAT, REPEAT_MASK}};

// @todo: ajouter un bouton reset + save.
// @todo: ajouter un bouton pour voir les réglages (velo, note, cc) dans le lcd.

bool midiCCIsActive = false;
bool ultrasonicSensorIsActive = false;
bool noteRepeatIsActive = false;
bool encoderSwitch1isActive = false;
bool encoderSwitch2isActive = false;
uint8_t rightPush = RELEASED;
uint8_t leftPush = RELEASED;
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
unsigned long oneNoteTime = 0;
unsigned long stopTime = 0;

uint8_t pushPin[NB_PUSH] = {4, 3, 2, 5, 6, 7, 8, 9};
byte pushNote[NB_PUSH];
byte pushVelocity[NB_PUSH] = {100, 100, 100, 100, 100, 100, 100, 100};
bool pushSettingsLocked[NB_PUSH] = {false, false, false, false, false, false, false, false};
byte pushRepeatSpeed[NB_PUSH][2] = {{1,4}, {1,4}, {1,4}, {1,4}, {1,4}, {1,4}, {1,4}, {1,4}};
// Nb elapsed repeats timeframes (not necessarily used) since last start time:
unsigned long pushElapsedRepeats[NB_PUSH] = {0, 0, 0, 0, 0, 0, 0, 0};
byte isPushed[NB_PUSH] = {RELEASED, RELEASED, RELEASED, RELEASED, RELEASED, RELEASED, RELEASED, RELEASED};
byte repeatIsLocked[NB_PUSH] = {false, false, false, false, false, false, false, false};
int selectedPushPin = -1;

void reset() {
  midiCC1 = 0;
  midiCC2 = 0;
  midiCC1Value = 63;
  midiCC2Value = 63;
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
  playButtonPressed = false;
  repeatSpeedDividend = 1;
  repeatSpeedDivisor = 4;
  globalStartNote = 48;

  for (int i = 0; i < 8; i++) {
    pushVelocity[i] = 100;
  }
  for (int i = 0; i < 8; i++) {
    pushSettingsLocked[i] = false;
  }
  for (int i = 0; i < 8; i++) {
    pushRepeatSpeed[i][0] = 1;
    pushRepeatSpeed[i][1] = 4;
  }
  for (int i = 0; i < 8; i++) {
    pushElapsedRepeats[i] = 0;
  }
  for (int i = 0; i < 8; i++) {
    isPushed[i] = RELEASED;
  }
  for (int i = 0; i < 8; i++) {
    repeatIsLocked[i] = false;
  }
  selectedPushPin = -1;
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
  for (uint8_t pad = 0; pad < NB_PUSH; pad++) {
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

  // if (checkMode(STOP_MASK)) {
  //   return;
  // }

  // LCD:
  display.begin();
  display.print("P0AH");
  delay(1000);
  if (checkMode(STOP_MASK)) {
    readSwitches();
    if (checkMode(STOP_MASK)) {
      display.blink();
      readSwitches();
      if (checkMode(STOP_MASK)) {
        display.print("P0AH PLAY");
        readSwitches();
        if (checkMode(STOP_MASK)) {
          display.snake(2, 70);
        }
      }
    }
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

  // Maj des réglages de niveau 1 (faders + encoders):
  if (checkMode(CC_MASK)) {
    updateMidiControls();
  }
  else {
    updateVelocityAndNotes();
  }

  // Maj des réglages de niveau 2 (symetric encoder push):
  if (checkMode(REPEAT_MASK)) {
    updateNoteRepeatSpeed();
  }
  else if (checkMode(ULTRASONIC_MASK)) {
    updateUltrasonicSettings();
  }
  else if (checkMode(CC_MASK)) {
    selectMidiControls();
  }
  else {
    updateDefaultGlobalSettings();
  }

  // Maj des réglages de niveau 3 (play button):
  if (!checkMode(STOP_MASK) && playFlag == false) {
    // @todo send keyboard key press ?
    // Serial.write(0XFA);
    // MIDI.sendStop();
    // displayPrintString("PLAY");
    // Reset:
    if (leftPush == PUSHED) {
      displayPrintString("init");
      reset();
    }
  }

  // Maj des réglages de niveau 4 (pad lock):
  if (checkMode(REPEAT_MASK)) {
    updatePadsRepeatLock();
  }
  else if (checkMode(CC_MASK)) {
    setCCPreset();
  }
  else {
    updatePadsLock();
  }
}

void updateDefaultGlobalSettings() {
  encoderValue = readEncoder(0);
  if (rightPush == PUSHED && encoderValue != encoderPos[0]) {
    int newMidiChannel = getMidiValueFromEncoder(midiChannel, encoderValue, encoderPos[0]);
    midiChannel = newMidiChannel > 16 ? 17 : newMidiChannel;
    displayPrintString("CH" + String(midiChannel));
    encoderPos[0] = encoderValue;
  }
  encoderValue = readEncoder(1);
  if (leftPush == PUSHED && encoderValue != encoderPos[1]) {
    programChange = getMidiValueFromEncoder(programChange, encoderValue, encoderPos[1]);
    MIDI.sendProgramChange(programChange, midiChannel);
    displayPrintString("P" + String(programChange));
    encoderPos[1] = encoderValue;
  }
}

void updateVelocityAndNotes() {

  // @todo note presets + gammes

  faderValue = readFader(0);
  if (faderValue != faderPos[0] && rightPush == RELEASED) {
    faderPos[0] = faderValue;
    uint8_t newMidiValue = getMidiValueFromFader(faderPos[0]);
    updateVelocity(newMidiValue, newMidiValue);
  }

  faderValue = readFader(1);
  if (faderValue != faderPos[1] && leftPush == RELEASED) {
    // float pitchBend = getPitchModulationFromfader(faderValue);
    // MIDI.sendPitchBend(0.5f, midiChannel);
    // displayPrintFloat(pitchBend);
    uint16_t newMidiValue = getMidiValueFromFader(faderValue);
    updateNotes(newMidiValue-60, newMidiValue);
    faderPos[1] = faderValue;
  }

  encoderValue = readEncoder(0);
  if (rightPush == RELEASED && encoderValue != encoderPos[0]) {
    updateVelocity(
      getMidiValueFromEncoder(globalVelocity, encoderValue, encoderPos[0]),
      getMidiValueFromEncoder(pushVelocity[selectedPushPin], encoderValue, encoderPos[0])
    );
    encoderPos[0] = encoderValue;
  }
  encoderValue = readEncoder(1);
  if (leftPush == RELEASED && encoderValue != encoderPos[1]) {
    updateNotes(
      (getMidiValueFromEncoder(60+globalNoteOffset, encoderValue, encoderPos[1]) - 60),
      getMidiValueFromEncoder(pushNote[selectedPushPin], encoderValue, encoderPos[1])
    );
    encoderPos[1] = encoderValue;
  }
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
    display.clear();
    display.print("CONT");
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
  }
  if (serialByte == MIDI_STOP) {
    playFlag = false;
    stopTime = micros();
    display.clear();
    display.print("STOP");
  }
  if (serialByte == MIDI_SONG_POSITION_POINTER) {
    // @todo.
    display.clear();
    display.print("POS");
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

uint16_t getMidiValueFromFader(unsigned long faderValue) {
  if (faderValue >= MAX_FADER_VALUE) {
    faderValue = 1024;
  }
  if (faderValue <= MIN_FADER_VALUE) {
    faderValue = 0;
  }
  return 127 - ((faderValue * 127) / 1024);
}

float getPitchModulationFromfader(uint16_t faderValue) {
  if (faderValue > 512) {
    return (float) (faderValue - 512) / 512;
  }
  else {
    return (float) faderValue / 512 * -1;
  }
}

int getMidiValueFromEncoder(uint8_t currentMidiValue, int position, int previousPosition) {
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

int readFader(uint8_t f) {
  int faderValue = analogRead(faderPin[f]);
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
    displayPrintString("v" + String(localMidiValue));
  }
  else {
    globalVelocity = globalMidiValue;
    pushVelocity[selectedPushPin] = globalVelocity;
    displayPrintString("v" + String(globalVelocity));
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

void updatePadsLock() {
  for (uint8_t p = 0; p < NB_PUSH; p++) {

    mux.channel(pushPin[p]);
    uint8_t sensorVal = digitalRead(MUXSIG);
    if (sensorVal == PUSHED && isPushed[p] == RELEASED) {
      if (leftPush == PUSHED) {
        pushSettingsLocked[p] = true;
        displayPrintString("PAd" + String(p+1));
      }
      else if (rightPush == PUSHED) {
        pushSettingsLocked[p] = false;
        displayPrintString("Glob");
      }
      else if (selectedPushPin != p && pushSettingsLocked[p] == true) {
        displayPrintInt(pushVelocity[p]);
      }
      else if (selectedPushPin != p && pushSettingsLocked[p] == false) {
        displayPrintInt(globalVelocity);
      }
    }
  }
}

void updatePadsRepeatLock() {

  for (uint8_t p = 0; p < NB_PUSH; p++) {

    mux.channel(pushPin[p]);
    uint8_t sensorVal = digitalRead(MUXSIG);

    if (leftPush == PUSHED && sensorVal == PUSHED && repeatIsLocked[p] == false) {
      repeatIsLocked[p] = true;
      displayPrintString("LoK" + String(p+1));
    }
    else if (rightPush == PUSHED && sensorVal == PUSHED && repeatIsLocked[p] == true) {
      isPushed[p] = RELEASED;
      repeatIsLocked[p] = false;
      displayPrintString("ULoK");
    }
  }
}

void updatePads() {

  for (uint8_t p = 0; p < NB_PUSH; p++) {

    mux.channel(pushPin[p]);
    uint8_t sensorVal = digitalRead(MUXSIG);

    if (sensorVal == PUSHED && isPushed[p] == RELEASED) {

      isPushed[p] = PUSHED;
      selectedPushPin = p;

      if (!checkMode(CC_MASK) || (rightPush == RELEASED && leftPush == RELEASED)) {
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
  if (!checkMode(REPEAT_MASK)) {
    return;
  }
  for (uint8_t pin = 0; pin < NB_PUSH; pin++) {
    unsigned long nextCap = getNextRepeatMicros(pin);
    if (micros() > nextCap) {
      pushElapsedRepeats[pin]++;
      if (isPushed[pin] == PUSHED || repeatIsLocked[pin] == true) {
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
  float pinRepeatSpeed = getPushPinFraction(pin);
  unsigned long oneNoteFractionMicros = getOneNoteFractionMicros(pinRepeatSpeed);
  return startTime + (pushElapsedRepeats[pin] * oneNoteFractionMicros) + oneNoteFractionMicros;
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
      displayPrintInt(bpm);
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

void selectMidiControls() {
  if (leftPush == PUSHED) {
    encoderValue = readEncoder(1);
    if (encoderValue != encoderPos[1]) {
      midiCC1 = getMidiValueFromEncoder(midiCC1, encoderValue, encoderPos[1]);
      encoderPos[1] = encoderValue;
      displayPrintString("C" + String(midiCC1));
    }
  }
  if (rightPush == PUSHED) {
    encoderValue = readEncoder(0);
    if (encoderValue != encoderPos[0]) {
      midiCC2 = getMidiValueFromEncoder(midiCC2, encoderValue, encoderPos[0]);
      displayPrintString("C" + String(midiCC2));
      encoderPos[0] = encoderValue;
    }
  }
}

void updateMidiControls() {

  encoderValue = readEncoder(0);
  if (rightPush == RELEASED && encoderValue != encoderPos[0]) {
    midiCC1Value = getMidiValueFromEncoder(midiCC1Value, encoderValue, encoderPos[0]);
    MIDI.sendControlChange(midiCC1, midiCC1Value, midiChannel);
    encoderPos[0] = encoderValue;
    displayPrintString("c" + String(midiCC1Value));
  }
  encoderValue = readEncoder(1);
  if (leftPush == RELEASED && encoderValue != encoderPos[1]) {
    midiCC2Value = getMidiValueFromEncoder(midiCC2Value, encoderValue, encoderPos[1]);
    MIDI.sendControlChange(midiCC2, midiCC2Value, midiChannel);
    encoderPos[1] = encoderValue;
    displayPrintString("c" + String(midiCC2Value));
  }

  faderValue = readFader(0);
  if (faderValue != faderPos[0]) {
    faderPos[0] = faderValue;
    midiCC1Value = getMidiValueFromFader(faderPos[0]);
    MIDI.sendControlChange(midiCC1, midiCC1Value, midiChannel);
    displayPrintString("c" + String(midiCC1Value));
  }
  faderValue = readFader(1);
  if (faderValue != faderPos[1]) {
    faderPos[1] = faderValue;
    midiCC2Value = getMidiValueFromFader(faderPos[1]);
    MIDI.sendControlChange(midiCC2, midiCC2Value, midiChannel);
    displayPrintString("c" + String(midiCC2Value));
  }
}

void updateNoteRepeatSpeed() {

  bool changed = false;
  uint8_t tmpRepeatSpeedDivisor = repeatSpeedDivisor;
  if (pushSettingsLocked[selectedPushPin]) {
    tmpRepeatSpeedDivisor = pushRepeatSpeed[selectedPushPin][1];
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
      pushRepeatSpeed[selectedPushPin][1] = tmpRepeatSpeedDivisor;
    }
    else {
      repeatSpeedDivisor = tmpRepeatSpeedDivisor;
    }

    float newSpeedFraction = (float) 1 / (float) tmpRepeatSpeedDivisor;
    unsigned long relativeStartTime = micros() - startTime;

    for (uint8_t pin = 0; pin < NB_PUSH; pin++) {
      if ((pushSettingsLocked[selectedPushPin] || repeatIsLocked[selectedPushPin]) && selectedPushPin != pin) {
        continue;
      }
      pushElapsedRepeats[pin] = ceil((float) relativeStartTime / (float) getOneNoteFractionMicros(newSpeedFraction)) + 1;
    }

    display.clear();
    display.print(" 1" + String(tmpRepeatSpeedDivisor));
    display.setColonOn(true);
  }
}

void updateUltrasonicSettings() {
  encoderValue = readEncoder(0);
  if (rightPush == PUSHED && encoderValue != encoderPos[0]) {
    ultrasonicCC = getMidiValueFromEncoder(ultrasonicCC, encoderValue, encoderPos[0]);
    displayPrintString("C" + String(ultrasonicCC));
    encoderPos[0] = encoderValue;
  }
  encoderValue = readEncoder(1);
  if (leftPush == PUSHED && encoderValue != encoderPos[1]) {
    maxUltrasonicDistanceCm = maxUltrasonicDistanceCm + (encoderValue - encoderPos[1]);
    displayPrintString("d" + String(maxUltrasonicDistanceCm));
    encoderPos[1] = encoderValue;
  }
  uint8_t distance = distanceSensor.measureDistanceCm();
  distance = distance > maxUltrasonicDistanceCm ? maxUltrasonicDistanceCm : distance;
  distance = distance < MIN_ULTRASONIC_DISTANCE_CM ? MIN_ULTRASONIC_DISTANCE_CM : distance;
  float distancePercentage = 1 - ((float) (distance - MIN_ULTRASONIC_DISTANCE_CM) / (float) (maxUltrasonicDistanceCm - MIN_ULTRASONIC_DISTANCE_CM));
  uint8_t ultrasonicControlValue = distancePercentage * 127;
  MIDI.sendControlChange(ultrasonicCC, ultrasonicControlValue, midiChannel);
}

void setCCPreset() {
  for (uint8_t p = 0; p < NB_PUSH; p++) {

    mux.channel(pushPin[p]);
    uint8_t sensorVal = digitalRead(MUXSIG);

    if (sensorVal == RELEASED) {
      continue;
    }

    if (leftPush == PUSHED) {
      midiCC1 = midiCCPresets[p];
      displayPrintString("P" + String(midiCCPresets[p]));
    }
    else if (rightPush == PUSHED) {
      midiCC2 = midiCCPresets[p];
      displayPrintString("P" + String(midiCCPresets[p]));
    }
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

bool checkMode(byte mask) {
  return ((currentPlayMode & mask) == mask);
}
