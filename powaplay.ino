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
#include "src/profiles/selected_profiles.h"
#include "src/platform/midi_iface.h"
#include "src/hal/io_iface.h"
#include "src/hal/display_iface.h"
#include "src/hal/hardware_cleanup.h"

const unsigned long BAUD_RATE PROGMEM = 38400;

// Misc

const byte UNASSIGNED PROGMEM = 255;

// Push buttons:

const byte PUSHED PROGMEM = LOW;
const byte RELEASED PROGMEM = HIGH;
const byte NB_PUSH PROGMEM = 8;

// Encoders:

const byte NB_ENCODERS PROGMEM = 2;

const byte P1CLK PROGMEM = kSelectedWiringProfile.encoders[0].clk.id;
const byte P1DT PROGMEM = kSelectedWiringProfile.encoders[0].dt.id;
const SignalRef P1SW = kSelectedWiringProfile.encoders[0].sw;
const byte P2CLK PROGMEM = kSelectedWiringProfile.encoders[1].clk.id;
const byte P2DT PROGMEM = kSelectedWiringProfile.encoders[1].dt.id;
const SignalRef P2SW = kSelectedWiringProfile.encoders[1].sw;

Encoder P1(P1CLK, P1DT);
Encoder P2(P2CLK, P2DT);

Encoder* encoder[NB_ENCODERS] = {&P1, &P2};
// Old state:
int encoderPos[NB_ENCODERS] = {0, 0};
// New state:
int encoderVal[NB_ENCODERS] = {0, 0};

const byte ENCODER_STEP PROGMEM = 4;

// Multiplexer

const byte MUX1SIG PROGMEM = kSelectedWiringProfile.mux1Sig.id;
const byte MUX2SIG PROGMEM = kSelectedWiringProfile.mux2Sig.id;
CD74HC4067 mux1(kSelectedWiringProfile.mux1S0.id, kSelectedWiringProfile.mux1S1.id, kSelectedWiringProfile.mux1S2.id, kSelectedWiringProfile.mux1S3.id);
CD74HC4067 mux2(kSelectedWiringProfile.mux2S0.id, kSelectedWiringProfile.mux2S1.id, kSelectedWiringProfile.mux2S2.id, kSelectedWiringProfile.mux2S3.id);

// MIDI

byte midiCC[2] = {
  kSelectedMappingProfile.defaultCcLane1,
  kSelectedMappingProfile.defaultCcLane2,
};
byte midiCCValue[2] = {
  kSelectedMappingProfile.defaultCcValueLane1,
  kSelectedMappingProfile.defaultCcValueLane2,
};
byte lastSentCCNumber[2] = {255, 255};
byte lastSentCCValue[2] = {255, 255};
// https://professionalcomposers.com/midi-cc-list/
// 5, 7, 10, 71, 72, 73, 74, 80, 81, 84, 91, 92, 93, 94, 95 - 98-101.
const byte midiCCPresets[NB_PUSH] PROGMEM = {
  kSelectedMappingProfile.midiCcPresets[0],
  kSelectedMappingProfile.midiCcPresets[1],
  kSelectedMappingProfile.midiCcPresets[2],
  kSelectedMappingProfile.midiCcPresets[3],
  kSelectedMappingProfile.midiCcPresets[4],
  kSelectedMappingProfile.midiCcPresets[5],
  kSelectedMappingProfile.midiCcPresets[6],
  kSelectedMappingProfile.midiCcPresets[7]
};
byte midiChannel = 2;
byte programChange = 0;
byte globalVelocity = 127;
int globalNoteOffset = 0;
const byte ticksPerNote PROGMEM = 96;
const byte REPEAT_SPEED_DIVISORS[] = {2, 4, 8, 16, 32, 64};
const byte NB_REPEAT_SPEED_DIVISORS = sizeof(REPEAT_SPEED_DIVISORS) / sizeof(REPEAT_SPEED_DIVISORS[0]);
unsigned long startTime = 0;
long nbElapsedNotes = 0;
const int MIDI_START_OFFSET PROGMEM = 0;
unsigned long lastClockPulse = 0;
unsigned long lastNoteRepeat = 0;

// LCD

const byte LCD_CLK PROGMEM = kSelectedWiringProfile.lcd.clk.id;
const byte LCD_DIO PROGMEM = kSelectedWiringProfile.lcd.dio.id;

SevenSegmentFun display(LCD_CLK, LCD_DIO);

// Faders:

const SignalRef faderPin[2] = {kSelectedWiringProfile.faders[0], kSelectedWiringProfile.faders[1]};
const byte NB_FADERS PROGMEM = 2;
uint16_t faderPos[NB_FADERS] = {0, 0};
uint16_t faderVal[NB_FADERS] = {0, 0};

const uint16_t MAX_FADER_VALUE = kSelectedMappingProfile.faderMaxValue;
const uint16_t MIN_FADER_VALUE = kSelectedMappingProfile.faderMinValue;
const uint16_t FADER_THRESHOLD = kSelectedMappingProfile.faderThreshold;
const uint16_t FADER_DEADBAND = kSelectedMappingProfile.faderDeadband;
const uint16_t FADER_EMA_ALPHA_PERMILLE = kSelectedMappingProfile.faderEmaAlphaPermille;
const unsigned long FADER_CC_MIN_UPDATE_INTERVAL_US = kSelectedMappingProfile.faderCcMinUpdateIntervalUs;
const unsigned long PAD_DEBOUNCE_US = kSelectedMappingProfile.padDebounceUs;
hardware_cleanup::AnalogState faderCleanupState[NB_FADERS] = {
  hardware_cleanup::makeAnalogState(),
  hardware_cleanup::makeAnalogState()
};
unsigned long lastFaderCcSentMicros[NB_FADERS] = {0, 0};

// Leds:

const byte L1 PROGMEM = kSelectedWiringProfile.leds[0].id;
const byte L2 PROGMEM = kSelectedWiringProfile.leds[1].id;
const byte L3 PROGMEM = kSelectedWiringProfile.leds[2].id;
const byte L4 PROGMEM = kSelectedWiringProfile.leds[3].id;

// Ultrasonic

const byte triggerPin PROGMEM = kSelectedWiringProfile.ultrasonic.trigger.id;
const byte echoPin PROGMEM = kSelectedWiringProfile.ultrasonic.echo.id;

UltraSonicDistanceSensor distanceSensor(triggerPin, echoPin);

byte MIN_ULTRASONIC_DISTANCE_CM = kSelectedMappingProfile.minUltrasonicDistanceCm;
int maxUltrasonicDistanceCm = kSelectedMappingProfile.defaultUltrasonicMaxDistanceCm;
byte ultrasonicCC = kSelectedMappingProfile.defaultUltrasonicCc;
byte lastUltrasonicControlValue = 255;
const int MAX_ULTRASONIC_DISTANCE_CAP_CM = kSelectedMappingProfile.maxUltrasonicDistanceCapCm;
const float ULTRASONIC_SMOOTHING_ALPHA = kSelectedMappingProfile.ultrasonicSmoothingAlpha;
const byte ULTRASONIC_CC_DEADBAND = kSelectedMappingProfile.ultrasonicCcDeadband;
const unsigned long ULTRASONIC_MIN_UPDATE_INTERVAL_US = kSelectedMappingProfile.ultrasonicMinUpdateIntervalUs;
int ultrasonicMedianBuffer[3] = {0, 0, 0};
byte ultrasonicMedianCount = 0;
byte ultrasonicMedianIndex = 0;
int lastValidUltrasonicDistanceCm = -1;
float smoothedUltrasonicDistanceCm = -1.0;
unsigned long lastUltrasonicUpdateMicros = 0;

// Magnet

const byte MAGNET PROGMEM = kSelectedWiringProfile.magnetPin.id;

// Switches

const SignalRef SW_CC = kSelectedWiringProfile.swCcChannel;
const SignalRef SW_REPEAT = kSelectedWiringProfile.swRepeatChannel;
const SignalRef SW_ULTRASONIC = kSelectedWiringProfile.swUltrasonicChannel;
const SignalRef SW_PLAY = kSelectedWiringProfile.swPlayChannel;

const byte INIT_MASK PROGMEM =       B00000010;
const byte CC_MASK PROGMEM =         B00000100;
const byte REPEAT_MASK PROGMEM =     B00001000;
const byte ULTRASONIC_MASK PROGMEM = B00010000;

byte currentPlayMode = B00000000;

enum class RuntimeMode : uint8_t {
  Play = 0,
  Cc,
  Repeat,
  Ultrasonic,
};

struct InputState {
  bool cc;
  bool repeat;
  bool ultrasonic;
  bool init;
  bool leftPush;
  bool rightPush;
};

struct ModeContext {
  RuntimeMode activeMode;
  bool anyEncoderPush;
  bool ccActive;
  bool repeatActive;
  bool ultrasonicActive;
  bool allowFreeEncoder;
};

InputState currentInputState = {false, false, false, false, false, false};
RuntimeMode selectedMode = RuntimeMode::Play;
RuntimeMode activeMode = RuntimeMode::Play;
RuntimeMode previousDisplayedMode = RuntimeMode::Play;
bool forceModeLabelDisplay = false;
bool modeScrollPending = false;
RuntimeMode modeScrollMode = RuntimeMode::Play;
unsigned long modeScrollPendingStartMs = 0;
display_iface::NonBlockingScrollState modeScrollState;
uint32_t modeScrollBaselineMutation = 0;
const unsigned long MODE_SCROLL_DELAY_MS = 0;
const unsigned long MODE_SCROLL_STEP_MS = 120;
display_iface::NonBlockingScrollState lockFeedbackScrollState;
uint32_t lockFeedbackBaselineMutation = 0;
bool suppressPadNoteDisplay = false;

struct ModeSwitchRef {
  SignalRef signal;
  byte mask;
};

ModeSwitchRef switches[4] = {
  {SW_CC, CC_MASK},
  {SW_PLAY, INIT_MASK},
  {SW_ULTRASONIC, ULTRASONIC_MASK},
  {SW_REPEAT, REPEAT_MASK}
};
bool prevCcSwitchOn = false;
bool prevRepeatSwitchOn = false;
bool prevUltrasonicSwitchOn = false;
bool prevPlaySwitchOn = false;

bool midiCCIsActive = false;
bool ultrasonicSensorIsActive = false;
bool encoderSwitch1isActive = false;
bool encoderSwitch2isActive = false;
byte rightPush = RELEASED;
byte leftPush = RELEASED;
bool prevLeftPushActive = false;
bool prevRightPushActive = false;
bool leftPushRisingEdge = false;
bool rightPushRisingEdge = false;
bool resetGestureAwaitingSecondPush = false;
unsigned long resetGestureFirstPushMs = 0;
const unsigned long FACTORY_RESET_DOUBLE_PUSH_INTERVAL_MS = 600;

bool isActiveLevel(byte rawValue, uint8_t activeLevel) {
  if (activeLevel == INPUT_ACTIVE_LOW) {
    return rawValue == LOW;
  }
  return rawValue == HIGH;
}

byte readDigitalSignalStable(SignalRef signal) {
  if (isMux1Source(signal)) {
    mux1.channel(signal.id);
    delayMicroseconds(3);
    return io_iface::readDigital(MUX1SIG);
  }
  if (isMux2Source(signal)) {
    if (!isUsableSignalPin(kSelectedWiringProfile.mux2Sig)) {
      return RELEASED;
    }
    mux2.channel(signal.id);
    delayMicroseconds(3);
    return io_iface::readDigital(MUX2SIG);
  }
  return io_iface::readDigital(signal.id);
}

int readAnalogSignalStable(SignalRef signal) {
  if (isMux1Source(signal)) {
    mux1.channel(signal.id);
    delayMicroseconds(3);
    return io_iface::readAnalog(MUX1SIG);
  }
  if (isMux2Source(signal)) {
    if (!isUsableSignalPin(kSelectedWiringProfile.mux2Sig)) {
      return 0;
    }
    mux2.channel(signal.id);
    delayMicroseconds(3);
    return io_iface::readAnalog(MUX2SIG);
  }
  return io_iface::readAnalog(signal.id);
}

bool isModeSwitchActive(byte rawValue) {
  return isActiveLevel(rawValue, APP_MODE_SWITCH_ACTIVE_LEVEL);
}

bool isEncoderPushActive(byte rawValue) {
  return isActiveLevel(rawValue, APP_ENCODER_PUSH_ACTIVE_LEVEL);
}

void reinit();

ModeContext deriveModeContext(const InputState& input) {
  bool ccRising = input.cc && !prevCcSwitchOn;
  bool repeatRising = input.repeat && !prevRepeatSwitchOn;
  bool ultrasonicRising = input.ultrasonic && !prevUltrasonicSwitchOn;
  bool playRising = input.init && !prevPlaySwitchOn;

  if (ccRising) {
    selectedMode = RuntimeMode::Cc;
    forceModeLabelDisplay = true;
  }
  if (repeatRising) {
    selectedMode = RuntimeMode::Repeat;
    forceModeLabelDisplay = true;
  }
  if (ultrasonicRising) {
    selectedMode = RuntimeMode::Ultrasonic;
    forceModeLabelDisplay = true;
  }
  if (playRising) {
    selectedMode = RuntimeMode::Play;
    forceModeLabelDisplay = true;
  }

  activeMode = selectedMode;

  prevCcSwitchOn = input.cc;
  prevRepeatSwitchOn = input.repeat;
  prevUltrasonicSwitchOn = input.ultrasonic;
  prevPlaySwitchOn = input.init;

  ModeContext context = {
    activeMode,
    input.leftPush || input.rightPush,
    activeMode == RuntimeMode::Cc,
    activeMode == RuntimeMode::Repeat,
    activeMode == RuntimeMode::Ultrasonic,
    false,
  };
  context.allowFreeEncoder = !context.anyEncoderPush;
  return context;
}

bool consumeFactoryResetDoublePushGesture(bool leftRising, bool rightRising, unsigned long nowMs) {
  bool pushRising = leftRising || rightRising;

  if (resetGestureAwaitingSecondPush && (nowMs - resetGestureFirstPushMs > FACTORY_RESET_DOUBLE_PUSH_INTERVAL_MS)) {
    resetGestureAwaitingSecondPush = false;
  }

  if (!pushRising) {
    return false;
  }

  if (resetGestureAwaitingSecondPush) {
    resetGestureAwaitingSecondPush = false;
    return true;
  }

  resetGestureAwaitingSecondPush = true;
  resetGestureFirstPushMs = nowMs;
  return false;
}

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
byte repeatArpStepNumber = 8;
byte repeatSpeedDividend = 1;
byte repeatSpeedDivisor = 4;
byte globalStartNote = 48;
unsigned long oneNoteTime = 0;
unsigned long stopTime = 0;
int octave = 0;

const byte NB_ARP_TYPES PROGMEM = 11;
const byte ARP_TYPE_SINGLE_NOTE PROGMEM = 0;
const byte ARP_TYPE_UP PROGMEM = 1;
const byte ARP_TYPE_DOWN PROGMEM = 2;
const byte ARP_TYPE_DOWN_UP PROGMEM = 3;
const byte ARP_TYPE_UP_DOWN PROGMEM = 4;
const byte ARP_TYPE_RANDOM PROGMEM = 5;
const byte ARP_TYPE_RANDOM_NOREPEAT PROGMEM = 6;
const byte ARP_TYPE_SHUFFLE PROGMEM = 7;
const byte ARP_TYPE_CONVERGE PROGMEM = 8;
const byte ARP_TYPE_ORDER PROGMEM = 9;
const byte ARP_TYPE_ASSIGN PROGMEM = 10;
const byte MAX_NOTES PROGMEM = 8;
byte selectedArpType = ARP_TYPE_SINGLE_NOTE;
const char kArpName0[] PROGMEM = "NOtE";
const char kArpName1[] PROGMEM = " UP ";
const char kArpName2[] PROGMEM = " dn ";
const char kArpName3[] PROGMEM = " dU ";
const char kArpName4[] PROGMEM = " Ud ";
const char kArpName5[] PROGMEM = "rAnd";
const char kArpName6[] PROGMEM = "rnNo";
const char kArpName7[] PROGMEM = "SHFL";
const char kArpName8[] PROGMEM = "CNVr";
const char kArpName9[] PROGMEM = "Ordr";
const char kArpName10[] PROGMEM = "ASGN";
const char* const ARP_NAMES[NB_ARP_TYPES] PROGMEM = {
  kArpName0,
  kArpName1,
  kArpName2,
  kArpName3,
  kArpName4,
  kArpName5,
  kArpName6,
  kArpName7,
  kArpName8,
  kArpName9,
  kArpName10
};

const SignalRef pushPin[NB_PUSH] = {
  kSelectedWiringProfile.pushPins[0],
  kSelectedWiringProfile.pushPins[1],
  kSelectedWiringProfile.pushPins[2],
  kSelectedWiringProfile.pushPins[3],
  kSelectedWiringProfile.pushPins[4],
  kSelectedWiringProfile.pushPins[5],
  kSelectedWiringProfile.pushPins[6],
  kSelectedWiringProfile.pushPins[7],
};
byte pushNote[NB_PUSH];
byte pushVelocity[NB_PUSH] = {100, 100, 100, 100, 100, 100, 100, 100};
bool pushSettingsLocked[NB_PUSH] = {false, false, false, false, false, false, false, false};
byte pushRepeatSpeed[NB_PUSH][2] = {{1,4}, {1,4}, {1,4}, {1,4}, {1,4}, {1,4}, {1,4}, {1,4}};
byte pushRepeatArpType[NB_PUSH] = {ARP_TYPE_SINGLE_NOTE, ARP_TYPE_SINGLE_NOTE, ARP_TYPE_SINGLE_NOTE, ARP_TYPE_SINGLE_NOTE, ARP_TYPE_SINGLE_NOTE, ARP_TYPE_SINGLE_NOTE, ARP_TYPE_SINGLE_NOTE, ARP_TYPE_SINGLE_NOTE};
byte pushRepeatStepNumber[NB_PUSH] = {8,8,8,8,8,8,8,8};
byte liveRepeatSpeedDivisor[NB_PUSH] = {4,4,4,4,4,4,4,4};
byte pendingRepeatSpeedDivisor[NB_PUSH] = {0,0,0,0,0,0,0,0};
bool pendingRepeatSpeedChange[NB_PUSH] = {false,false,false,false,false,false,false,false};
// Track last NoteOn per pad so NoteOff always matches, even if octave/scale changes while held.
byte padActiveNote[NB_PUSH] = {0,0,0,0,0,0,0,0};
bool padNoteIsOn[NB_PUSH] = {false,false,false,false,false,false,false,false};
// For arp playback: schedule a NoteOff some microseconds after NoteOn to create a real gate.
unsigned long padScheduledOffMicros[NB_PUSH] = {0,0,0,0,0,0,0,0};
// Nb elapsed arp timeframes since last start time.
unsigned long pushElapsedRepeats[NB_PUSH] = {0, 0, 0, 0, 0, 0, 0, 0};
byte arpStepProgress[NB_PUSH] = {0,0,0,0,0,0,0,0};
byte arpSlotIndex[NB_PUSH] = {255,255,255,255,255,255,255,255};
int arpDirection[NB_PUSH] = {1,1,1,1,1,1,1,1};
byte arpLastRandomIndex[NB_PUSH] = {255,255,255,255,255,255,255,255};
byte arpShuffleOrder[NB_PUSH][MAX_NOTES] = {
  {0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0}
};
byte arpShuffleCount[NB_PUSH] = {0,0,0,0,0,0,0,0};
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
hardware_cleanup::DigitalDebounceState padDebounceState[NB_PUSH] = {
  hardware_cleanup::makeDigitalDebounceState(),
  hardware_cleanup::makeDigitalDebounceState(),
  hardware_cleanup::makeDigitalDebounceState(),
  hardware_cleanup::makeDigitalDebounceState(),
  hardware_cleanup::makeDigitalDebounceState(),
  hardware_cleanup::makeDigitalDebounceState(),
  hardware_cleanup::makeDigitalDebounceState(),
  hardware_cleanup::makeDigitalDebounceState()
};
unsigned long pushedTime[NB_PUSH] = {0,0,0,0,0,0,0,0};
bool repeatIsLocked[NB_PUSH] = {false, false, false, false, false, false, false, false};
bool repeatSettingsLocked[NB_PUSH] = {false, false, false, false, false, false, false, false};
int selectedPushPin = -1;
unsigned long padPressOrder[NB_PUSH] = {0,0,0,0,0,0,0,0};
unsigned long nextPadPressOrder = 1;

// Chords
// Including NOTE (no chord).
const byte NB_CHORDS PROGMEM = 14;
byte selectedChord = 0;
const char kChordName0[] PROGMEM = "NOTE";
const char kChordName1[] PROGMEM = "MAJ ";
const char kChordName2[] PROGMEM = "MIN ";
const char kChordName3[] PROGMEM = "AUG ";
const char kChordName4[] PROGMEM = "dIM ";
const char kChordName5[] PROGMEM = "SUS2";
const char kChordName6[] PROGMEM = "SUS4";
const char kChordName7[] PROGMEM = " 7th";
const char kChordName8[] PROGMEM = "MAJ7";
const char kChordName9[] PROGMEM = "MIN7";
const char kChordName10[] PROGMEM = "d7  ";
const char kChordName11[] PROGMEM = "5th ";
const char kChordName12[] PROGMEM = "Ad9 ";
const char kChordName13[] PROGMEM = "m7b6";
const char* const CHORD_NAMES[NB_CHORDS] PROGMEM = {
  kChordName0,
  kChordName1,
  kChordName2,
  kChordName3,
  kChordName4,
  kChordName5,
  kChordName6,
  kChordName7,
  kChordName8,
  kChordName9,
  kChordName10,
  kChordName11,
  kChordName12,
  kChordName13
};

// Scales
const byte NB_SCALES PROGMEM = 12;
const byte SCALE_INDEX_SEMI PROGMEM = 0;
const byte SCALE_INDEX_DRUM PROGMEM = 8;
const byte SCALE_INDEX_PAG8 PROGMEM = 11;
byte selectedScale = 0;
const char kScaleName0[] PROGMEM = "SEMI";
const char kScaleName1[] PROGMEM = "MAJ";
const char kScaleName2[] PROGMEM = "MIN_";
const char kScaleName3[] PROGMEM = "BLUE";
const char kScaleName4[] PROGMEM = "BLU_";
const char kScaleName5[] PROGMEM = "PENT";
const char kScaleName6[] PROGMEM = "DOR ";
const char kScaleName7[] PROGMEM = "JAPN";
const char kScaleName8[] PROGMEM = "DRUM";
const char kScaleName9[] PROGMEM = "MIX ";
const char kScaleName10[] PROGMEM = "JAZZ";
const char kScaleName11[] PROGMEM = "PAG8";
const char* const SCALE_NAMES[NB_SCALES] PROGMEM = {
  kScaleName0,
  kScaleName1,
  kScaleName2,
  kScaleName3,
  kScaleName4,
  kScaleName5,
  kScaleName6,
  kScaleName7,
  kScaleName8,
  kScaleName9,
  kScaleName10,
  kScaleName11
};
const byte SCALES[NB_SCALES][MAX_NOTES] PROGMEM = {
  {0, 1, 2, 3, 4, 5, 6, 7}, // Semitone
  {0, 2, 4, 5, 7, 9, 11, 12}, // Major
  {0, 2, 3, 5, 7, 8, 10, 12}, // Minor
  {0, 2, 3, 4, 7, 9, UNASSIGNED, UNASSIGNED}, // Blues (major-blues style)
  {0, 3, 5, 6, 7, 10, UNASSIGNED, UNASSIGNED}, // Blues minor
  {0, 2, 4, 7, 9, 12, 14, 16}, // Major pentatonic
  {0, 2, 3, 5, 7, 9, 10, 12}, // Dorian
  {0, 1, 5, 7, 8, 12, 13, 17}, // Japanese / In Sen style
  {0, 2, 6, 10, 5, 9, 13, 15}, // DRUM parity row (runtime uses fixed DRUM_NOTES_MELODICS)
  {0, 2, 4, 5, 7, 9, 10, 12}, // Mixolydian
  {0, 2, 4, 5, 7, 9, 10, 11}, // Jazz (Bebop dominant)
  {0, 1, 2, 3, 4, 5, 6, 7}, // PAG8 chromatic page; octave/page moves by 8 semitones
};
// Melodics-compatible GM drum map:
// KICK(36), SNARE(38), CHH(42), OHH(46), LOW TOM(41), MID TOM(45), CRASH(49), RIDE(51)
const byte DRUM_NOTES_MELODICS[MAX_NOTES] PROGMEM = {
  36,  // Pad 1 drum note: Kick
  38,  // Pad 2 drum note: Snare
  42,  // Pad 3 drum note: Closed Hi-Hat
  46,  // Pad 4 drum note: Open Hi-Hat
  41,  // Pad 5 drum note: Low Tom
  45,  // Pad 6 drum note: Mid Tom
  49,  // Pad 7 drum note: Crash
  51,  // Pad 8 drum note: Ride
};
const byte CHORDS[NB_CHORDS][MAX_NOTES] PROGMEM = {
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
  {0, 3, 6, 9, UNASSIGNED, UNASSIGNED, UNASSIGNED, UNASSIGNED},           // dim7
  {0, 7, UNASSIGNED, UNASSIGNED, UNASSIGNED, UNASSIGNED, UNASSIGNED, UNASSIGNED}, // 5th
  {0, 4, 7, 14, UNASSIGNED, UNASSIGNED, UNASSIGNED, UNASSIGNED},          // add9
  {0, 3, 7, 10, 20, UNASSIGNED, UNASSIGNED, UNASSIGNED},                  // m7b6
};

void resetArpState(byte pin, unsigned long referenceTime = 0);
void resetAllArpStates(unsigned long referenceTime = 0);
bool isPadArpActive(byte pin);
void syncHeldPadsAfterNoteLayoutChange();
bool getPadPlaybackState(byte pin, byte &currentNote, byte &currentVelocity);
bool triggerPadRootNote(byte sourcePad, byte note, byte velocity, bool state);
bool getDirectionalArpPlaybackState(byte pin, byte arpType, byte stepCount, byte &currentNote, byte &currentVelocity);
byte getArpSourcePad(byte sourcePad, byte arpType);
byte getOrderedArpAnchorPad();
void buildArpSlotPads(byte sourcePad, byte arpPads[MAX_NOTES], byte &validCount, byte &startPos);
void buildOrderedActivePads(byte sourcePad, byte arpPads[MAX_NOTES], byte &validCount, byte &startPos, bool pinStart);

byte readScaleStep(byte scaleIndex, byte padIndex) {
  byte directStep = pgm_read_byte(&SCALES[scaleIndex][padIndex]);
  if (directStep != UNASSIGNED) {
    return directStep;
  }

  byte compactSteps[MAX_NOTES];
  byte validCount = 0;
  for (byte i = 0; i < MAX_NOTES; i++) {
    byte step = pgm_read_byte(&SCALES[scaleIndex][i]);
    if (step == UNASSIGNED) {
      continue;
    }
    compactSteps[validCount++] = step;
  }

  if (validCount == 0) {
    return UNASSIGNED;
  }

  byte wrappedIndex = padIndex % validCount;
  byte octaveOffset = (padIndex / validCount) * 12;
  int expandedStep = compactSteps[wrappedIndex] + octaveOffset;
  if (expandedStep > 127) {
    expandedStep = 127;
  }
  return (byte) expandedStep;
}

byte readDrumNote(byte padIndex) {
  return pgm_read_byte(&DRUM_NOTES_MELODICS[padIndex]);
}

int getOctaveStrideSemitones() {
  return selectedScale == SCALE_INDEX_PAG8 ? 8 : 12;
}

byte readChordInterval(byte chordIndex, byte intervalIndex) {
  return pgm_read_byte(&CHORDS[chordIndex][intervalIndex]);
}

void printProgmemLabel(const char* const* labels, byte index) {
  char label[6];
  const char* labelPtr = (const char*) pgm_read_word(&labels[index]);
  strncpy_P(label, labelPtr, sizeof(label) - 1);
  label[sizeof(label) - 1] = '\0';
  display_iface::print(label);
}

const char* modeLabel(RuntimeMode mode) {
  switch (mode) {
    case RuntimeMode::Play:
      return "PLAy";
    case RuntimeMode::Cc:
      return "CC";
    case RuntimeMode::Repeat:
      return "rEPEAt";
    case RuntimeMode::Ultrasonic:
      return "ULtrA SonIC";
  }
  return "PLAy";
}

void showModeIfChanged(RuntimeMode mode, bool suppressDisplay) {
  if (mode == previousDisplayedMode && !forceModeLabelDisplay) {
    return;
  }

  previousDisplayedMode = mode;
  if (suppressDisplay) {
    forceModeLabelDisplay = false;
    modeScrollPending = false;
    display_iface::cancelScroll(modeScrollState);
    return;
  }

  forceModeLabelDisplay = false;
  modeScrollMode = mode;
  modeScrollPending = true;
  modeScrollPendingStartMs = millis();
  modeScrollBaselineMutation = display_iface::getMutationCounter();
  display_iface::cancelScroll(modeScrollState);
}

void tickModeScroll(bool interactionActive) {
  if (!modeScrollPending && !modeScrollState.active) {
    return;
  }

  if (interactionActive) {
    modeScrollPending = false;
    display_iface::cancelScroll(modeScrollState);
    return;
  }

  if (display_iface::getMutationCounter() != modeScrollBaselineMutation) {
    modeScrollPending = false;
    display_iface::cancelScroll(modeScrollState);
    return;
  }

  if (modeScrollPending) {
    unsigned long elapsed = millis() - modeScrollPendingStartMs;
    if (elapsed < MODE_SCROLL_DELAY_MS) {
      return;
    }
    modeScrollPending = false;
    display_iface::startScroll(modeScrollState, modeLabel(modeScrollMode), MODE_SCROLL_STEP_MS);
    modeScrollBaselineMutation = display_iface::getMutationCounter();
  }
  display_iface::tickScroll(modeScrollState);
}

void queueLockFeedback(bool isLocked, byte pad) {
  char label[16];
  strcpy(label, isLocked ? "LOC P" : "ULOC P");
  label[isLocked ? 5 : 6] = '1' + pad;
  strcpy(label + (isLocked ? 6 : 7), " SET");
  modeScrollPending = false;
  display_iface::cancelScroll(modeScrollState);
  display_iface::startScroll(lockFeedbackScrollState, label, MODE_SCROLL_STEP_MS);
  lockFeedbackBaselineMutation = display_iface::getMutationCounter();
}

void queueRepeatLockFeedback(bool isLocked, byte pad) {
  char label[24];
  strcpy(label, isLocked ? "LOC P" : "ULOC P");
  label[isLocked ? 5 : 6] = '1' + pad;
  strcpy(label + (isLocked ? 6 : 7), " rEP");
  modeScrollPending = false;
  display_iface::cancelScroll(modeScrollState);
  display_iface::startScroll(lockFeedbackScrollState, label, MODE_SCROLL_STEP_MS);
  lockFeedbackBaselineMutation = display_iface::getMutationCounter();
}

void queueResetFeedback() {
  modeScrollPending = false;
  display_iface::cancelScroll(modeScrollState);
  display_iface::startScroll(lockFeedbackScrollState, "rEsEt", MODE_SCROLL_STEP_MS);
  lockFeedbackBaselineMutation = display_iface::getMutationCounter();
}

void updateHeldPadsRepeatLock(bool isLocked) {
  for (byte p = 0; p < NB_PUSH; p++) {
    if (isPushed[p] != PUSHED) {
      continue;
    }
    if (isLocked && !repeatIsLocked[p]) {
      repeatIsLocked[p] = true;
    }
    else if (!isLocked) {
      bool wasLocked = repeatIsLocked[p];
      repeatIsLocked[p] = false;
      if (wasLocked) {
        padScheduledOffMicros[p] = 0;
        if (padNoteIsOn[p]) {
          sendNote(padActiveNote[p], 0, false);
          padNoteIsOn[p] = false;
        }
        resetArpState(p);
      }
    }
    queueRepeatLockFeedback(repeatIsLocked[p], p);
  }
}

void updateHeldPadsSettingsLock(bool isLocked) {
  bool anyChanged = false;
  for (byte p = 0; p < NB_PUSH; p++) {
    if (isPushed[p] != PUSHED) {
      continue;
    }
    if (isLocked) {
      if (!pushSettingsLocked[p]) {
        lockPadPlaySettings(p);
        pushSettingsLocked[p] = true;
        anyChanged = true;
      }
      if (!repeatSettingsLocked[p]) {
        resetPadRepeatToGlobal(p);
        repeatSettingsLocked[p] = true;
      }
    }
    else {
      if (pushSettingsLocked[p]) {
        pushSettingsLocked[p] = false;
        resetPadToGlobalSettings(p);
        anyChanged = true;
      }
      if (repeatSettingsLocked[p]) {
        repeatSettingsLocked[p] = false;
        resetPadRepeatToGlobal(p);
      }
    }
    queueLockFeedback(pushSettingsLocked[p], p);
  }
  if (anyChanged) {
    syncHeldPadsAfterNoteLayoutChange();
  }
}

void resetPadToGlobalSettings(byte pad) {
  pushNote[pad] = globalStartNote;
  pushVelocity[pad] = globalVelocity;
}

void lockPadPlaySettings(byte pad) {
  bool wasLocked = pushSettingsLocked[pad];
  pushSettingsLocked[pad] = false;
  byte currentNote = 0;
  byte currentVelocity = 0;
  if (getPadPlaybackState(pad, currentNote, currentVelocity)) {
    pushNote[pad] = currentNote;
    pushVelocity[pad] = currentVelocity;
  }
  pushSettingsLocked[pad] = wasLocked;
}

bool usesLocalRepeatSettings(byte pad) {
  return repeatSettingsLocked[pad];
}

void resetPadRepeatToGlobal(byte pad) {
  pushRepeatSpeed[pad][0] = 1;
  pushRepeatSpeed[pad][1] = repeatSpeedDivisor;
  pushRepeatArpType[pad] = selectedArpType;
  pushRepeatStepNumber[pad] = repeatArpStepNumber;
  pendingRepeatSpeedDivisor[pad] = repeatSpeedDivisor;
  pendingRepeatSpeedChange[pad] = false;
  liveRepeatSpeedDivisor[pad] = repeatSpeedDivisor;
}

void tickLockFeedback(bool interactionActive) {
  if (!lockFeedbackScrollState.active) {
    return;
  }
  if (interactionActive && !suppressPadNoteDisplay) {
    display_iface::cancelScroll(lockFeedbackScrollState);
    return;
  }
  if (display_iface::getMutationCounter() != lockFeedbackBaselineMutation) {
    display_iface::cancelScroll(lockFeedbackScrollState);
    return;
  }
  display_iface::tickScroll(lockFeedbackScrollState);
}

void reinit() {
  midiCC[0] = kSelectedMappingProfile.defaultCcLane1;
  midiCC[1] = kSelectedMappingProfile.defaultCcLane2;
  midiCCValue[0] = kSelectedMappingProfile.defaultCcValueLane1;
  midiCCValue[1] = kSelectedMappingProfile.defaultCcValueLane2;
  lastSentCCNumber[0] = 255;
  lastSentCCNumber[1] = 255;
  lastSentCCValue[0] = 255;
  lastSentCCValue[1] = 255;
  midiChannel = 2;
  programChange = 0;
  globalVelocity = 127;
  globalNoteOffset = 0;
  ultrasonicCC = kSelectedMappingProfile.defaultUltrasonicCc;
  maxUltrasonicDistanceCm = kSelectedMappingProfile.defaultUltrasonicMaxDistanceCm;
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
  encoderSwitch1isActive = false;
  encoderSwitch2isActive = false;
  rightPush = RELEASED;
  leftPush = RELEASED;
  prevLeftPushActive = false;
  prevRightPushActive = false;
  leftPushRisingEdge = false;
  rightPushRisingEdge = false;
  resetGestureAwaitingSecondPush = false;
  resetGestureFirstPushMs = 0;
  display_iface::cancelScroll(lockFeedbackScrollState);
  lockFeedbackBaselineMutation = 0;
  selectedMode = RuntimeMode::Play;
  activeMode = RuntimeMode::Play;
  previousDisplayedMode = RuntimeMode::Play;
  repeatSpeedDividend = 1;
  repeatSpeedDivisor = 4;
  repeatArpStepNumber = 8;
  globalStartNote = 48;

  for (byte i = 0; i < 8; i++) {
    pushVelocity[i] = 100;
    pushNote[i] = globalStartNote;
  }
  for (byte i = 0; i < 8; i++) {
    pushSettingsLocked[i] = false;
    repeatSettingsLocked[i] = false;
  }
  for (byte i = 0; i < 8; i++) {
    pushRepeatSpeed[i][0] = 1;
    pushRepeatSpeed[i][1] = 4;
    pushRepeatArpType[i] = ARP_TYPE_SINGLE_NOTE;
    pushRepeatStepNumber[i] = 8;
    liveRepeatSpeedDivisor[i] = 4;
    pendingRepeatSpeedDivisor[i] = 0;
    pendingRepeatSpeedChange[i] = false;
  }
  for (byte i = 0; i < 8; i++) {
    pushElapsedRepeats[i] = 0;
  }
  for (byte i = 0; i < 8; i++) {
    arpSlotIndex[i] = UNASSIGNED;
  }
  for (byte i = 0; i < 8; i++) {
    arpDirection[i] = 1;
  }
  for (byte i = 0; i < 8; i++) {
    arpLastRandomIndex[i] = UNASSIGNED;
    arpShuffleCount[i] = 0;
    padPressOrder[i] = 0;
  }
  for (byte i = 0; i < 8; i++) {
    isPushed[i] = RELEASED;
    padDebounceState[i] = hardware_cleanup::makeDigitalDebounceState();
  }
  for (byte i = 0; i < 8; i++) {
    repeatIsLocked[i] = false;
  }
  selectedPushPin = -1;
  octave = 0;
  selectedChord = 0;
  selectedArpType = ARP_TYPE_SINGLE_NOTE;
  nextPadPressOrder = 1;

  for (byte i = 0; i < 8; i++) {
    padNoteIsOn[i] = false;
    padActiveNote[i] = 0;
  }
  for (byte i = 0; i < NB_FADERS; i++) {
    faderCleanupState[i] = hardware_cleanup::makeAnalogState();
    lastFaderCcSentMicros[i] = 0;
  }
}

void appSetupImpl();
void appLoopImpl();

void setup() { appSetupImpl(); }
void loop() { appLoopImpl(); }

void appSetupImpl() {

#if APP_MIDI_TRANSPORT == MIDI_TRANSPORT_SERIAL
  Serial.begin(BAUD_RATE);
#endif
  app_midi::begin();

#if APP_MIDI_TRANSPORT == MIDI_TRANSPORT_SERIAL
  Serial.print("MIDI P0AH PLAY (Serial MIDI debug) --- Compiled on ");
  Serial.print(__DATE__);
  Serial.print(" at ");
  Serial.println(__TIME__);
  Serial.println();
#endif

  // Push
  for (byte pad = 0; pad < NB_PUSH; pad++) {
    pushNote[pad] = globalStartNote;
  }

  // Encoders:
  io_iface::setPinMode(P1CLK, INPUT);
  io_iface::setPinMode(P1DT, INPUT);
  io_iface::setPinMode(P2CLK, INPUT);
  io_iface::setPinMode(P2DT, INPUT);

  // Leds:
  io_iface::setPinMode(L1, OUTPUT);
  io_iface::setPinMode(L2, OUTPUT);
  io_iface::setPinMode(L3, OUTPUT);
  io_iface::setPinMode(L4, OUTPUT);

  // Magnet:
  io_iface::setPinMode(MAGNET, OUTPUT);
  io_iface::writeDigital(MAGNET, HIGH);

  // mux sig:
  io_iface::setPinMode(MUX1SIG, INPUT_PULLUP);
  if (isUsableSignalPin(kSelectedWiringProfile.mux2Sig)) {
    io_iface::setPinMode(MUX2SIG, INPUT_PULLUP);
  }

  // Internal led:
  io_iface::setPinMode(LED_BUILTIN, OUTPUT);
  io_iface::writeDigital(LED_BUILTIN, LOW);

  // MIDI Clock:
  quarterNoteTime = micros();
  oneNoteTime = getNoteMicros();

  byte bootPlayRaw = readDigitalSignalStable(SW_PLAY);
  bool bootPlaySwitchOn = isModeSwitchActive(bootPlayRaw);

  readSwitches();

  // LCD:
  display_iface::init(display);
  display_iface::begin();
  if (bootPlaySwitchOn) {
    display_iface::setPrintDelay(150);
    display_iface::scrollingText("P0AH PLAY", 1);
    display_iface::setPrintDelay(TM1637_DEFAULT_PRINT_DELAY);
  }
  else {
    display_iface::print("P0AH PLAY");
    display_iface::blink();
    display_iface::snake(2, 70);
  }
}

void appLoopImpl() {

  updateMidiSerial();
  processScheduledNoteOffs();
  if (playFlag == false) {
    playPadsArp();
  }

  readSwitches();
  ModeContext modeContext = deriveModeContext(currentInputState);
  bool leftPushActive = (leftPush == PUSHED);
  bool rightPushActive = (rightPush == PUSHED);
  leftPushRisingEdge = leftPushActive && !prevLeftPushActive;
  rightPushRisingEdge = rightPushActive && !prevRightPushActive;
  prevLeftPushActive = leftPushActive;
  prevRightPushActive = rightPushActive;

  if (consumeFactoryResetDoublePushGesture(leftPushRisingEdge, rightPushRisingEdge, millis())) {
    panicAllNotesOff();
    reinit();
    queueResetFeedback();
    return;
  }

  for (byte pos = 0; pos < NB_FADERS; pos++) {
    faderVal[pos] = readFader(pos);
  }
  for (byte pos = 0; pos < NB_ENCODERS; pos++) {
    encoderVal[pos] = readEncoder(pos);
  }

  bool anyEncoderMovement = false;
  for (byte pos = 0; pos < NB_ENCODERS; pos++) {
    if (encoderVal[pos] != encoderPos[pos]) {
      anyEncoderMovement = true;
      break;
    }
  }
  suppressPadNoteDisplay = modeContext.anyEncoderPush || anyEncoderMovement || lockFeedbackScrollState.active;

  updatePads();
  io_iface::writeDigital(MAGNET, aPadIsPushed() ? LOW : HIGH);

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

  bool interactionActive = modeContext.anyEncoderPush || aPadIsPushed();
  bool modeDisplaySuppressed = interactionActive;
  for (byte pos = 0; pos < NB_FADERS && !modeDisplaySuppressed; pos++) {
    modeDisplaySuppressed = (faderVal[pos] != faderPos[pos]);
  }
  for (byte pos = 0; pos < NB_ENCODERS && !modeDisplaySuppressed; pos++) {
    modeDisplaySuppressed = (encoderVal[pos] != encoderPos[pos]);
  }
  showModeIfChanged(modeContext.activeMode, modeDisplaySuppressed);
  tickModeScroll(modeDisplaySuppressed);
  tickLockFeedback(modeDisplaySuppressed);

  // Encoders et faders (sans encoder push)
  if (modeContext.ccActive) {
    updateCCValueFromFader(0);
    updateCCValueFromFader(1);

    // In CC mode, a push on either encoder reserves interaction context,
    // so both free-encoder CC-value updates are paused.
    if (modeContext.allowFreeEncoder) {
      updateMidiCCValueFromEncoder(0);
      updateMidiCCValueFromEncoder(1);
    }
  }
  else {
    if (modeContext.ultrasonicActive) {
      updateCCValueFromFader(0);
      updateCCValueFromFader(1);
    }
    else {
      updateVelocityFromFader(0);
      updateOctaveFromFader(1);
    }

    if (modeContext.allowFreeEncoder) {
      if (modeContext.activeMode == RuntimeMode::Play) {
        scaleSelect(0);
        updateBaseNoteFromEncoder(1);
      }
      else if (modeContext.activeMode == RuntimeMode::Repeat) {
        arpSelect(0);
        updateArpRateFromEncoder(1);
      }
      else if (modeContext.activeMode == RuntimeMode::Ultrasonic) {
        updateUltrasonicCC(0);
        updateUltrasonicDistance(1);
      }
    }
  }

  // Encoder push
  if (modeContext.ultrasonicActive) {
    // Ultrasonic edits are available on free encoder rotation.
  }
  else if (modeContext.repeatActive) {
    if (leftPush == PUSHED) {
      if (leftPushRisingEdge && aPadIsPushed()) {
        updateHeldPadsRepeatLock(true);
      }
      else {
        updateArpStepNumberFromEncoder(0);
        updatePreciseArpRateFromEncoder(1);
      }
    }
    if (rightPush == PUSHED) {
      if (rightPushRisingEdge && aPadIsPushed()) {
        updateHeldPadsRepeatLock(false);
      }
      else {
        updateBpmFromEncoder(1);
      }
    }
  }
  else if (modeContext.ccActive) {
    if (leftPush == PUSHED) {
      updateMidiControlFromEncoder(0);
      selectCCPreset(0);
    }
    if (rightPush == PUSHED) {
      updateMidiControlFromEncoder(1);
      selectCCPreset(1);
    }
  }
  else {
    if (leftPush == PUSHED) {
      if (leftPushRisingEdge && aPadIsPushed()) {
        updateHeldPadsSettingsLock(true);
      }
      else {
        updateChannelFromEncoder(0);
      }
    }
    if (rightPush == PUSHED) {
      if (rightPushRisingEdge && aPadIsPushed()) {
        updateHeldPadsSettingsLock(false);
      }
      else {
        chordSelect(1);
      }
    }
  }

  if (modeContext.ultrasonicActive) {
    trackUltrasonicChanges();
  }

  app_midi::flush();
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
    display_iface::clear();
    display_iface::setColonOn(false);
    char label[6] = "CH";
    itoa(midiChannel, label + 2, 10);
    display_iface::print(label);
    encoderPos[selected] = encoderVal[selected];
  }
}

bool noteBelongsToCurrentScale(int noteValue) {
  if (selectedScale == SCALE_INDEX_SEMI) {
    return true;
  }
  if (selectedScale == SCALE_INDEX_DRUM) {
    return true;
  }

  int normalized = (noteValue - (60 + globalNoteOffset)) % 12;
  if (normalized < 0) {
    normalized += 12;
  }

  for (byte pad = 0; pad < MAX_NOTES; pad++) {
    byte step = readScaleStep(selectedScale, pad);
    if (step == UNASSIGNED) {
      continue;
    }
    if ((step % 12) == normalized) {
      return true;
    }
  }
  return false;
}

int stepNoteInCurrentScale(int currentNote, int direction) {
  if (selectedScale == SCALE_INDEX_SEMI) {
    int directNote = currentNote + direction;
    if (directNote < 0) {
      return 0;
    }
    if (directNote > 127) {
      return 127;
    }
    return directNote;
  }
  if (selectedScale == SCALE_INDEX_DRUM) {
    int directNote = currentNote + direction;
    if (directNote < 0) {
      return 0;
    }
    if (directNote > 127) {
      return 127;
    }
    return directNote;
  }

  int candidate = currentNote;
  while (true) {
    candidate += direction;
    if (candidate <= 0) {
      return 0;
    }
    if (candidate >= 127) {
      return 127;
    }
    if (noteBelongsToCurrentScale(candidate)) {
      return candidate;
    }
  }
}

void updateBaseNoteFromEncoder(byte selected) {
  if (encoderVal[selected] != encoderPos[selected]) {
    int delta = encoderVal[selected] - encoderPos[selected];
    int direction = delta > 0 ? 1 : -1;
    int steps = delta > 0 ? delta : -delta;
    bool hasLockedTarget = (selectedPushPin != -1 && pushSettingsLocked[selectedPushPin]);
    if (hasLockedTarget) {
      int localNote = pushNote[selectedPushPin];
      for (int i = 0; i < steps; i++) {
        localNote = stepNoteInCurrentScale(localNote, direction);
      }
      pushNote[selectedPushPin] = (byte) localNote;
      displayPrintString(getNoteFromMidiValue(pushNote[selectedPushPin]));
      syncHeldPadsAfterNoteLayoutChange();
    }
    else {
      int globalNote = 60 + globalNoteOffset;
      int localNote = selectedPushPin != -1 ? pushNote[selectedPushPin] : globalNote;
      for (int i = 0; i < steps; i++) {
        globalNote = stepNoteInCurrentScale(globalNote, direction);
        localNote = stepNoteInCurrentScale(localNote, direction);
      }
      updateNotes(globalNote - 60, localNote);
    }
  }
  encoderPos[selected] = encoderVal[selected];
}

void chordSelect(byte selected) {

  encoderVal[selected] = readEncoder(selected);
  if (encoderVal[selected] == encoderPos[selected]) {
    return;
  }

  int delta = encoderVal[selected] - encoderPos[selected];
  int wrappedChord = ((int) selectedChord + delta) % NB_CHORDS;
  if (wrappedChord < 0) {
    wrappedChord += NB_CHORDS;
  }
  selectedChord = wrappedChord;
  display_iface::clear();
  printProgmemLabel(CHORD_NAMES, selectedChord);
  display_iface::setColonOn(false);
  encoderPos[selected] = encoderVal[selected];
  syncHeldPadsAfterNoteLayoutChange();
}

void arpSelect(byte selected) {

  encoderVal[selected] = readEncoder(selected);
  if (encoderVal[selected] == encoderPos[selected]) {
    return;
  }

  int delta = encoderVal[selected] - encoderPos[selected];
  byte currentArpType = selectedArpType;
  if (selectedPushPin != -1 && usesLocalRepeatSettings(selectedPushPin)) {
    currentArpType = pushRepeatArpType[selectedPushPin];
  }
  int wrappedArpType = ((int) currentArpType + delta) % NB_ARP_TYPES;
  if (wrappedArpType < 0) {
    wrappedArpType += NB_ARP_TYPES;
  }
  if (selectedPushPin != -1 && usesLocalRepeatSettings(selectedPushPin)) {
    pushRepeatArpType[selectedPushPin] = wrappedArpType;
  }
  else {
    selectedArpType = wrappedArpType;
  }

  display_iface::clear();
  printProgmemLabel(ARP_NAMES, wrappedArpType);
  display_iface::setColonOn(false);
  encoderPos[selected] = encoderVal[selected];
  resetAllArpStates();
}

void updateOctaveFromFader(byte selected) {
  if (faderVal[selected] == faderPos[selected]) {
    return;
  }
  int previousOctave = octave;
  int relativeValue = (faderVal[selected] - 1024) * (-1);
  if (relativeValue >= MAX_FADER_VALUE) {
    relativeValue = 1024;
  }
  if (relativeValue <= MIN_FADER_VALUE) {
    relativeValue = 0;
  }
  octave = round(relativeValue / 100) - 4;
  if (!shouldSuppressLiveDisplay()) {
    display_iface::clear();
    char label[8];
    itoa(octave, label, 10);
    strcat(label, octave >= 0 ? "oct" : "oc");
    display_iface::print(label);
  }
  int octaveDelta = octave - previousOctave;
  int octaveStride = getOctaveStrideSemitones();
  bool hasLockedTarget = (selectedPushPin != -1 && pushSettingsLocked[selectedPushPin]);
  if (octaveDelta != 0 && hasLockedTarget) {
    int updatedNote = (int) pushNote[selectedPushPin] + (octaveDelta * octaveStride);
    if (updatedNote < 0) {
      updatedNote = 0;
    }
    if (updatedNote > 127) {
      updatedNote = 127;
    }
    pushNote[selectedPushPin] = (byte) updatedNote;
  }
  else if (octaveDelta != 0) {
    globalNoteOffset += octaveDelta * octaveStride;
    if (60 + globalNoteOffset < 0) {
      globalNoteOffset = -60;
    }
    if (60 + globalNoteOffset > 127) {
      globalNoteOffset = 67;
    }
  }
  faderPos[selected] = faderVal[selected];

  if (octaveDelta != 0) {
    // Live transposition: if pads are currently held, retrigger them
    // so the pitch follows the octave change.
    retriggerHeldPads();
  }
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

void updateMidiSerial() {
  uint8_t midiMessage = 0;
  while (app_midi::readRealtime(midiMessage)) {
    unsigned long loopTime = micros();

    if (midiMessage == MIDI_CONTINUE) {
      playFlag = true;
      startTime += micros() - stopTime;
      displayPrint("CONT", false, true);
      continue;
    }
    if (midiMessage == MIDI_START) {
      playFlag = true;
      startTime = loopTime - MIDI_START_OFFSET;
      nbElapsedNotes = 0;
      // Re-align beat phase on transport start.
      midiCLockTick = 0;
      resetAllArpStates(loopTime);
      displayPrint("PLAY", false, true);
      continue;
    }
    if (midiMessage == MIDI_STOP) {
      playFlag = false;
      stopTime = micros();
      displayPrint("STOP", false, true);
      continue;
    }
    if (midiMessage == MIDI_SONG_POSITION_POINTER) {
      // @todo.
      displayPrint("POS", false, true);
      continue;
    }
    if (midiMessage == MIDI_CLOCK && playFlag) {
      MidiSync();

      if (loopTime > getNextNoteMicros()) {
        nbElapsedNotes++;
      }

      playPadsArp();
    }
  }
}

void writeLeds(byte s1, byte s2, byte s3, byte s4) {
  byte ledPin[4] = {L1, L2, L3, L4};
  byte states[4] = {s1, s2, s3, s4};
  for (byte i = 0; i < 4; i++) {
    io_iface::writeDigital(ledPin[i], states[i]);
  }
}

void scaleSelect(byte selected) {
  encoderVal[selected] = readEncoder(selected);
  if (encoderVal[selected] == encoderPos[selected]) {
    return;
  }

  int delta = encoderVal[selected] - encoderPos[selected];
  int wrappedScale = ((int) selectedScale + delta) % NB_SCALES;
  if (wrappedScale < 0) {
    wrappedScale += NB_SCALES;
  }
  selectedScale = wrappedScale;
  display_iface::clear();
  printProgmemLabel(SCALE_NAMES, selectedScale);
  display_iface::setColonOn(false);
  encoderPos[selected] = encoderVal[selected];
  syncHeldPadsAfterNoteLayoutChange();
}

void playPush(byte pin, bool state) {
  byte currentVelocity = 0;
  byte currentNote = 0;
  if (!getPadPlaybackState(pin, currentNote, currentVelocity)) {
    if (!state && padNoteIsOn[pin]) {
      sendNote(padActiveNote[pin], 0, false);
      padNoteIsOn[pin] = false;
    }
    return;
  }
  triggerPadRootNote(pin, currentNote, currentVelocity, state);
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
  const char names[12] = {
    'C', 'd', 'D', 'E', 'E', 'F', 'F', 'G', 'A', 'A', 'b', 'b'
  };
  const char accidental[12] = {
    ' ', 'b', ' ', 'b', ' ', ' ', 'b', ' ', 'b', ' ', 'b', ' '
  };

  byte n = midiValue % 12;
  byte oct = midiValue / 12;
  // Keep same compact style as existing display strings (4 chars max)
  // e.g. " C4 ", "Eb4 ", "A10 " -> compressed for 4-char display.
  note[0] = names[n];
  if (oct < 10) {
    note[1] = accidental[n];
    note[2] = '0' + oct;
    note[3] = '\0';
  }
  else {
    // Two-digit octave fallback
    note[1] = '0' + oct / 10;
    note[2] = '0' + oct % 10;
    note[3] = '\0';
  }
  return note;
}

int readFader(byte selected) {
  uint16_t raw = readAnalogSignalStable(faderPin[selected]);
  hardware_cleanup::AnalogCleanupConfig config = {
    MIN_FADER_VALUE,
    MAX_FADER_VALUE,
    FADER_THRESHOLD,
    FADER_DEADBAND,
    FADER_EMA_ALPHA_PERMILLE,
  };
  return hardware_cleanup::conditionAnalog(raw, faderPos[selected], faderCleanupState[selected], config);
}

int readEncoder(byte e) {
  int position = encoder[e]->read() / ENCODER_STEP;
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
    if (!shouldSuppressLiveDisplay()) {
      display_iface::clear();
      display_iface::print('v');
      displayPrint(localMidiValue, false, false);
    }
  }
  else {
    globalVelocity = globalMidiValue;
    if (selectedPushPin != -1) {
      pushVelocity[selectedPushPin] = globalVelocity;
    }
    if (!shouldSuppressLiveDisplay()) {
      display_iface::clear();
      display_iface::print('v');
      displayPrint(globalVelocity, false, false);
    }
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
  syncHeldPadsAfterNoteLayoutChange();
}

void moveOctave(bool up) {
  int previousOctave = octave;
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
  display_iface::clear();
  char label[8];
  itoa(octave, label, 10);
  strcat(label, octave >= 0 ? "oct" : "oc");
  display_iface::print(label);
  display_iface::setColonOn(false);
  int octaveDelta = octave - previousOctave;
  int octaveStride = getOctaveStrideSemitones();
  bool hasLockedTarget = (selectedPushPin != -1 && pushSettingsLocked[selectedPushPin]);
  if (octaveDelta != 0 && hasLockedTarget) {
    int updatedNote = (int) pushNote[selectedPushPin] + (octaveDelta * octaveStride);
    if (updatedNote < 0) {
      updatedNote = 0;
    }
    if (updatedNote > 127) {
      updatedNote = 127;
    }
    pushNote[selectedPushPin] = (byte) updatedNote;
  }
  else if (octaveDelta != 0) {
    globalNoteOffset += octaveDelta * octaveStride;
    if (60 + globalNoteOffset < 0) {
      globalNoteOffset = -60;
    }
    if (60 + globalNoteOffset > 127) {
      globalNoteOffset = 67;
    }
  }

  if (octaveDelta != 0) {
    // Live transposition for encoder octave changes too.
    retriggerHeldPads();
  }
}

void retriggerHeldPads() {
  // For each currently-held pad: NoteOff the previously sounding note, then NoteOn the new one.
  // This avoids stuck notes and gives live transposition.
  syncHeldPadsAfterNoteLayoutChange();
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

void resetArpState(byte pin, unsigned long referenceTime) {
  if (referenceTime == 0) {
    referenceTime = micros();
  }
  pushedTime[pin] = referenceTime;
  pushElapsedRepeats[pin] = 0;
  arpStepProgress[pin] = 0;
  arpSlotIndex[pin] = UNASSIGNED;
  byte effectiveArpType = usesLocalRepeatSettings(pin) ? pushRepeatArpType[pin] : selectedArpType;
  arpDirection[pin] = (effectiveArpType == ARP_TYPE_DOWN || effectiveArpType == ARP_TYPE_DOWN_UP) ? -1 : 1;
  arpLastRandomIndex[pin] = UNASSIGNED;
  arpShuffleCount[pin] = 0;
}

void resetAllArpStates(unsigned long referenceTime) {
  if (referenceTime == 0) {
    referenceTime = micros();
  }
  for (byte p = 0; p < NB_PUSH; p++) {
    resetArpState(p, referenceTime);
  }
}

bool isPadArpActive(byte pin) {
  return repeatIsLocked[pin] == true || (activeMode == RuntimeMode::Repeat && isPushed[pin] == PUSHED);
}

void syncHeldPadsAfterNoteLayoutChange() {
  for (byte p = 0; p < NB_PUSH; p++) {
    if (!padNoteIsOn[p] && isPushed[p] != PUSHED && !repeatIsLocked[p]) {
      continue;
    }

    if (padNoteIsOn[p]) {
      sendNote(padActiveNote[p], 0, false);
      padNoteIsOn[p] = false;
    }
    padScheduledOffMicros[p] = 0;

    if (isPadArpActive(p)) {
      continue;
    }
    if (isPushed[p] == PUSHED) {
      playPush(p, true);
    }
  }
}

bool getPadPlaybackState(byte pin, byte &currentNote, byte &currentVelocity) {
  currentVelocity = pushVelocity[pin];

  if (selectedScale == SCALE_INDEX_DRUM) {
    // DRUM scale is intentionally a direct GM drum note map for Melodics/E-Drums compatibility.
    // Locked pads keep their stored note; unlocked pads follow the fixed drum map.
    currentVelocity = pushSettingsLocked[pin] ? pushVelocity[pin] : globalVelocity;
    currentNote = pushSettingsLocked[pin] ? pushNote[pin] : readDrumNote(pin);
    return true;
  }

  int noteValue = pushSettingsLocked[pin] ? pushNote[pin] : globalStartNote;
  if (!pushSettingsLocked[pin]) {
    currentVelocity = globalVelocity;
    noteValue += globalNoteOffset;
  }

  byte scaleStep = readScaleStep(selectedScale, pin);
  if (scaleStep == UNASSIGNED) {
    return false;
  }

  if (!pushSettingsLocked[pin]) {
    noteValue += scaleStep;
  }
  if (noteValue < 0) {
    noteValue = 0;
  }
  if (noteValue > 127) {
    noteValue = 127;
  }
  currentNote = noteValue;
  return true;
}

bool getDirectionalArpPlaybackState(byte pin, byte arpType, byte stepCount, byte &currentNote, byte &currentVelocity) {
  if (!getPadPlaybackState(pin, currentNote, currentVelocity)) {
    return false;
  }

  byte progress = arpStepProgress[pin];
  if (stepCount < 2) {
    stepCount = 2;
  }

  byte distance = 0;
  int direction = 1;

  if (arpType == ARP_TYPE_UP) {
    distance = progress % stepCount;
    direction = 1;
  }
  else if (arpType == ARP_TYPE_DOWN) {
    distance = progress % stepCount;
    direction = -1;
  }
  else {
    byte span = stepCount - 1;
    byte cycleLen = span * 2;
    byte cyclePos = progress % cycleLen;
    distance = (cyclePos <= span) ? cyclePos : (cycleLen - cyclePos);
    direction = (arpType == ARP_TYPE_UP_DOWN) ? 1 : -1;
  }

  for (byte i = 0; i < distance; i++) {
    currentNote = (byte) stepNoteInCurrentScale(currentNote, direction);
  }
  return true;
}

bool triggerPadRootNote(byte sourcePad, byte note, byte velocity, bool state) {
  if (state) {
    if (padNoteIsOn[sourcePad]) {
      sendNote(padActiveNote[sourcePad], 0, false);
      padNoteIsOn[sourcePad] = false;
    }

    padActiveNote[sourcePad] = note;
    padNoteIsOn[sourcePad] = true;
    sendNote(note, velocity, true);
    return true;
  }

  if (padNoteIsOn[sourcePad]) {
    sendNote(padActiveNote[sourcePad], 0, false);
    padNoteIsOn[sourcePad] = false;
  }
  return true;
}

byte getArpSourcePad(byte sourcePad, byte arpType) {
  byte arpPads[MAX_NOTES];
  byte validCount = 0;
  byte startPos = 0;
  if (arpType == ARP_TYPE_ORDER) {
    buildOrderedActivePads(sourcePad, arpPads, validCount, startPos, true);
  }
  else if (arpType == ARP_TYPE_ASSIGN) {
    buildOrderedActivePads(sourcePad, arpPads, validCount, startPos, false);
  }
  else {
    buildArpSlotPads(sourcePad, arpPads, validCount, startPos);
  }
  if (validCount == 0) {
    return UNASSIGNED;
  }

  if (arpType == ARP_TYPE_SINGLE_NOTE || validCount < 2) {
    return arpPads[startPos];
  }

  if (arpSlotIndex[sourcePad] == UNASSIGNED || arpSlotIndex[sourcePad] >= validCount) {
    arpSlotIndex[sourcePad] = startPos;
    arpDirection[sourcePad] = (arpType == ARP_TYPE_DOWN || arpType == ARP_TYPE_DOWN_UP) ? -1 : 1;
  }

  byte currentPos = arpSlotIndex[sourcePad];
  byte chosenPad = arpPads[currentPos];

  if (arpType == ARP_TYPE_UP) {
    arpSlotIndex[sourcePad] = (currentPos + 1) % validCount;
  }
  else if (arpType == ARP_TYPE_RANDOM) {
    chosenPad = arpPads[random(validCount)];
  }
  else if (arpType == ARP_TYPE_RANDOM_NOREPEAT) {
    byte randomPos = random(validCount);
    if (validCount > 1 && arpLastRandomIndex[sourcePad] != UNASSIGNED && randomPos == arpLastRandomIndex[sourcePad]) {
      randomPos = (randomPos + 1 + random(validCount - 1)) % validCount;
    }
    arpLastRandomIndex[sourcePad] = randomPos;
    chosenPad = arpPads[randomPos];
  }
  else if (arpType == ARP_TYPE_SHUFFLE) {
    if (arpShuffleCount[sourcePad] != validCount || arpSlotIndex[sourcePad] == UNASSIGNED) {
      for (byte i = 0; i < validCount; i++) {
        arpShuffleOrder[sourcePad][i] = i;
      }
      for (byte i = 0; i < validCount; i++) {
        byte swapIndex = i + random(validCount - i);
        byte tmp = arpShuffleOrder[sourcePad][i];
        arpShuffleOrder[sourcePad][i] = arpShuffleOrder[sourcePad][swapIndex];
        arpShuffleOrder[sourcePad][swapIndex] = tmp;
      }
      arpShuffleCount[sourcePad] = validCount;
      arpSlotIndex[sourcePad] = 0;
    }
    chosenPad = arpPads[arpShuffleOrder[sourcePad][arpSlotIndex[sourcePad]]];
    arpSlotIndex[sourcePad]++;
    if (arpSlotIndex[sourcePad] >= validCount) {
      arpSlotIndex[sourcePad] = UNASSIGNED;
    }
  }
  else if (arpType == ARP_TYPE_CONVERGE) {
    byte convergeStep = currentPos;
    byte leftIndex = convergeStep / 2;
    byte rightIndex = validCount - 1 - leftIndex;
    chosenPad = (convergeStep % 2 == 0) ? arpPads[leftIndex] : arpPads[rightIndex];
    arpSlotIndex[sourcePad] = (currentPos + 1) % validCount;
  }
  else {
    int nextPos = (int) currentPos + arpDirection[sourcePad];
    if (nextPos < 0 || nextPos >= validCount) {
      arpDirection[sourcePad] *= -1;
      nextPos = (int) currentPos + arpDirection[sourcePad];
    }
    arpSlotIndex[sourcePad] = nextPos;
  }

  return chosenPad;
}

byte getOrderedArpAnchorPad() {
  byte anchorPad = UNASSIGNED;
  unsigned long bestOrder = 0;

  for (byte p = 0; p < NB_PUSH; p++) {
    if (!isPadArpActive(p)) {
      continue;
    }
    if (padPressOrder[p] == 0) {
      continue;
    }
    if (anchorPad == UNASSIGNED || padPressOrder[p] < bestOrder) {
      anchorPad = p;
      bestOrder = padPressOrder[p];
    }
  }

  return anchorPad;
}

void buildArpSlotPads(byte sourcePad, byte arpPads[MAX_NOTES], byte &validCount, byte &startPos) {
  validCount = 0;
  startPos = 0;
  bool foundStart = false;

  for (byte p = 0; p < NB_PUSH; p++) {
    byte currentNote = 0;
    byte currentVelocity = 0;
    if (!getPadPlaybackState(p, currentNote, currentVelocity)) {
      continue;
    }
    arpPads[validCount] = p;
    if (!foundStart && p >= sourcePad) {
      startPos = validCount;
      foundStart = true;
    }
    validCount++;
  }

  if (validCount == 0) {
    return;
  }
  if (!foundStart) {
    startPos = 0;
  }
}

void buildOrderedActivePads(byte sourcePad, byte arpPads[MAX_NOTES], byte &validCount, byte &startPos, bool pinStart) {
  validCount = 0;
  startPos = 0;

  for (byte p = 0; p < NB_PUSH; p++) {
    if (padPressOrder[p] == 0) {
      continue;
    }
    byte currentNote = 0;
    byte currentVelocity = 0;
    if (!getPadPlaybackState(p, currentNote, currentVelocity)) {
      continue;
    }
    arpPads[validCount] = p;
    validCount++;
  }

  for (byte i = 0; i < validCount; i++) {
    byte best = i;
    for (byte j = i + 1; j < validCount; j++) {
      if (padPressOrder[arpPads[j]] < padPressOrder[arpPads[best]]) {
        best = j;
      }
    }
    if (best != i) {
      byte tmp = arpPads[i];
      arpPads[i] = arpPads[best];
      arpPads[best] = tmp;
    }
  }

  if (validCount == 0) {
    return;
  }

  if (!pinStart) {
    return;
  }

  for (byte i = 0; i < validCount; i++) {
    if (arpPads[i] == sourcePad) {
      startPos = i;
      return;
    }
  }
}

void panicAllNotesOff() {
  // Send All Notes Off (CC123) + All Sound Off (CC120), then clear local tracking.
  // This is a safety net if something ever gets stuck.
  app_midi::sendControlChange(123, 0, midiChannel);
  app_midi::sendControlChange(120, 0, midiChannel);

  for (byte p = 0; p < NB_PUSH; p++) {
    if (padNoteIsOn[p]) {
      sendNote(padActiveNote[p], 0, false);
    }
    padNoteIsOn[p] = false;
    padActiveNote[p] = 0;
    padScheduledOffMicros[p] = 0;
    isPushed[p] = RELEASED;
    repeatIsLocked[p] = false;
    repeatSettingsLocked[p] = false;
    padPressOrder[p] = 0;
    resetArpState(p);
  }
  nextPadPressOrder = 1;
}

void updatePadsLock(bool lock) {
  bool anyChanged = false;

  for (byte p = 0; p < NB_PUSH; p++) {

    if (isPushed[p] == PUSHED) {
      if (lock == false) {
        if (!pushSettingsLocked[p]) {
          lockPadPlaySettings(p);
          pushSettingsLocked[p] = true;
          anyChanged = true;
        }
        if (!repeatSettingsLocked[p]) {
          resetPadRepeatToGlobal(p);
          repeatSettingsLocked[p] = true;
        }
        queueLockFeedback(true, p);
      }
      else {
        if (pushSettingsLocked[p]) {
          pushSettingsLocked[p] = false;
          pushNote[p] = globalStartNote;
          pushVelocity[p] = globalVelocity;
          anyChanged = true;
        }
        if (repeatSettingsLocked[p]) {
          repeatSettingsLocked[p] = false;
          resetPadRepeatToGlobal(p);
        }
        queueLockFeedback(false, p);
      }
    }
  }
  if (anyChanged) {
    syncHeldPadsAfterNoteLayoutChange();
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
      resetArpState(p);
    }
    else {
      displayPrintString("Lock");
      padScheduledOffMicros[p] = 0;
    }
    repeatIsLocked[p] = isLocked;
  }
}

void updatePads() {

  for (byte p = 0; p < NB_PUSH; p++) {

    byte sensorRaw = readDigitalSignalStable(pushPin[p]);
    bool sensorActive = (sensorRaw == PUSHED);
    bool sensorDebouncedActive = hardware_cleanup::debounceDigital(
      sensorActive,
      micros(),
      PAD_DEBOUNCE_US,
      padDebounceState[p]
    );
    byte sensorVal = sensorDebouncedActive ? PUSHED : RELEASED;

    if (sensorVal == PUSHED && isPushed[p] == RELEASED) {
      byte orderedAnchorBeforePress = UNASSIGNED;
      if (selectedArpType == ARP_TYPE_ORDER || selectedArpType == ARP_TYPE_ASSIGN) {
        orderedAnchorBeforePress = getOrderedArpAnchorPad();
      }

      isPushed[p] = PUSHED;
      selectedPushPin = p;
      padPressOrder[p] = nextPadPressOrder++;
      resetArpState(p);

      if (activeMode != RuntimeMode::Cc || (rightPush == RELEASED && leftPush == RELEASED)) {
        pushedTime[p] = micros();
        bool encoderPushActive = (leftPush == PUSHED || rightPush == PUSHED);
        if (activeMode == RuntimeMode::Play && encoderPushActive) {
          bool shouldLock = (leftPush == PUSHED && rightPush != PUSHED);
          bool shouldUnlock = (rightPush == PUSHED && leftPush != PUSHED);
          bool changed = false;
          if (shouldLock) {
            if (!pushSettingsLocked[p]) {
              lockPadPlaySettings(p);
              pushSettingsLocked[p] = true;
              changed = true;
            }
            if (!repeatSettingsLocked[p]) {
              resetPadRepeatToGlobal(p);
              repeatSettingsLocked[p] = true;
            }
          }
          else if (shouldUnlock) {
            if (pushSettingsLocked[p]) {
              pushSettingsLocked[p] = false;
              resetPadToGlobalSettings(p);
              changed = true;
            }
            if (repeatSettingsLocked[p]) {
              repeatSettingsLocked[p] = false;
              resetPadRepeatToGlobal(p);
            }
          }
          if (shouldLock || shouldUnlock) {
            queueLockFeedback(pushSettingsLocked[p], p);
          }
          if (changed) {
            syncHeldPadsAfterNoteLayoutChange();
          }
        }
        else if (activeMode == RuntimeMode::Repeat && encoderPushActive) {
          bool shouldLock = (leftPush == PUSHED && rightPush != PUSHED);
          bool shouldUnlock = (rightPush == PUSHED && leftPush != PUSHED);
          bool changed = false;
          bool unlockedNow = false;
          if (shouldLock && !repeatIsLocked[p]) {
            repeatIsLocked[p] = true;
            changed = true;
          }
          else if (shouldUnlock) {
            unlockedNow = repeatIsLocked[p];
            repeatIsLocked[p] = false;
            changed = changed || unlockedNow;
          }
          if ((changed || unlockedNow) && !repeatIsLocked[p]) {
            padScheduledOffMicros[p] = 0;
            if (padNoteIsOn[p]) {
              sendNote(padActiveNote[p], 0, false);
              padNoteIsOn[p] = false;
            }
            resetArpState(p);
          }
          if (shouldLock || shouldUnlock) {
            queueRepeatLockFeedback(repeatIsLocked[p], p);
          }
        }
        bool shouldPreviewPad =
          activeMode != RuntimeMode::Repeat
          && !((selectedArpType == ARP_TYPE_ORDER || selectedArpType == ARP_TYPE_ASSIGN)
          && orderedAnchorBeforePress != UNASSIGNED);
        bool padArpActive = isPadArpActive(p);
        if (shouldPreviewPad && !padArpActive) {
          playPush(p, 1);
        }
        if (leftPush == RELEASED && rightPush == RELEASED && !suppressPadNoteDisplay) {
          byte currentNote = 0;
          byte currentVelocity = 0;
          if (getPadPlaybackState(p, currentNote, currentVelocity)) {
            displayPrintString(getNoteFromMidiValue(currentNote));
          }
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
      if (!repeatIsLocked[p]) {
        padPressOrder[p] = 0;
        resetArpState(p);
      }
      // Keep selectedPushPin sticky after release.
      // Selection changes only on the next pad press edge.
    }
  }
}

void playPadsArp() {
  byte orderedAnchorPad = UNASSIGNED;
  if (selectedArpType == ARP_TYPE_ORDER || selectedArpType == ARP_TYPE_ASSIGN) {
    orderedAnchorPad = getOrderedArpAnchorPad();
  }

  for (byte pin = 0; pin < NB_PUSH; pin++) {
    bool padArpActive = (activeMode == RuntimeMode::Repeat && isPushed[pin] == PUSHED) || repeatIsLocked[pin] == true;
    if (!padArpActive && pendingRepeatSpeedChange[pin]) {
      liveRepeatSpeedDivisor[pin] = pendingRepeatSpeedDivisor[pin];
      pendingRepeatSpeedChange[pin] = false;
      pendingRepeatSpeedDivisor[pin] = 0;
    }

    if (orderedAnchorPad != UNASSIGNED && (selectedArpType == ARP_TYPE_ORDER || selectedArpType == ARP_TYPE_ASSIGN) && pin != orderedAnchorPad) {
      if (!repeatIsLocked[pin] && padNoteIsOn[pin]) {
        sendNote(padActiveNote[pin], 0, false);
        padNoteIsOn[pin] = false;
        padScheduledOffMicros[pin] = 0;
      }
      continue;
    }
    if (!padArpActive) {
      padScheduledOffMicros[pin] = 0;
      if (
        (isPushed[pin] == RELEASED && repeatIsLocked[pin] == false) ||
        (activeMode != RuntimeMode::Repeat && repeatIsLocked[pin] == false)
      ) {
        resetArpState(pin, getNextRepeatMicros(pin));
      }
      continue;
    }
    unsigned long now = micros();
    unsigned long nextCap = getNextRepeatMicros(pin);
    const byte MAX_CATCHUP_STEPS = 8;
    byte catchupSteps = 0;
    while (nextCap != 0 && (long) (now - nextCap) >= 0 && catchupSteps < MAX_CATCHUP_STEPS) {
      if (
        (activeMode == RuntimeMode::Repeat && isPushed[pin] == PUSHED) ||
        repeatIsLocked[pin] == true
      ) {
        pushElapsedRepeats[pin]++;
        byte effectiveArpType = usesLocalRepeatSettings(pin) ? pushRepeatArpType[pin] : selectedArpType;
        byte currentNote = 0;
        byte currentVelocity = 0;
        byte stepCount = usesLocalRepeatSettings(pin) ? pushRepeatStepNumber[pin] : repeatArpStepNumber;
        bool hasPlaybackState = false;

        if (effectiveArpType == ARP_TYPE_UP ||
            effectiveArpType == ARP_TYPE_DOWN ||
            effectiveArpType == ARP_TYPE_UP_DOWN ||
            effectiveArpType == ARP_TYPE_DOWN_UP) {
          hasPlaybackState = getDirectionalArpPlaybackState(pin, effectiveArpType, stepCount, currentNote, currentVelocity);
        }
        else {
          byte arpPad = getArpSourcePad(pin, effectiveArpType);
          if (arpPad != UNASSIGNED) {
            hasPlaybackState = getPadPlaybackState(arpPad, currentNote, currentVelocity);
          }
        }

        if (hasPlaybackState) {
          byte stepProgress = arpStepProgress[pin];
          int steppedNote = currentNote;
          if (effectiveArpType != ARP_TYPE_UP &&
              effectiveArpType != ARP_TYPE_DOWN &&
              effectiveArpType != ARP_TYPE_UP_DOWN &&
              effectiveArpType != ARP_TYPE_DOWN_UP) {
            byte octaveLayer = (stepCount > MAX_NOTES) ? (stepProgress / MAX_NOTES) : 0;
            steppedNote = (int) currentNote + ((int) octaveLayer * getOctaveStrideSemitones());
          }
          if (steppedNote > 127) {
            steppedNote = 127;
          }
          triggerPadRootNote(pin, (byte) steppedNote, currentVelocity, true);
          stepProgress++;
          if (effectiveArpType == ARP_TYPE_SINGLE_NOTE) {
            stepProgress = 0;
          }
          else if (effectiveArpType == ARP_TYPE_UP ||
                   effectiveArpType == ARP_TYPE_DOWN ||
                   effectiveArpType == ARP_TYPE_UP_DOWN ||
                   effectiveArpType == ARP_TYPE_DOWN_UP) {
            byte directionalStepCount = stepCount < 2 ? 2 : stepCount;
            byte phraseLength = directionalStepCount;
            if (effectiveArpType == ARP_TYPE_UP_DOWN || effectiveArpType == ARP_TYPE_DOWN_UP) {
              phraseLength = (directionalStepCount - 1) * 2;
            }
            arpStepProgress[pin] = stepProgress % phraseLength;
          }
          else if (stepProgress >= stepCount) {
            stepProgress = 0;
            arpStepProgress[pin] = 0;
            arpSlotIndex[pin] = UNASSIGNED;
            arpDirection[pin] = (effectiveArpType == ARP_TYPE_DOWN || effectiveArpType == ARP_TYPE_DOWN_UP) ? -1 : 1;
            arpLastRandomIndex[pin] = UNASSIGNED;
            arpShuffleCount[pin] = 0;
          }
          else {
            arpStepProgress[pin] = stepProgress;
          }
          // Create a real gate time for arp notes.
          padScheduledOffMicros[pin] = now + getRepeatGateMicros(pin);
          if (pendingRepeatSpeedChange[pin]) {
            liveRepeatSpeedDivisor[pin] = pendingRepeatSpeedDivisor[pin];
            pendingRepeatSpeedChange[pin] = false;
            pendingRepeatSpeedDivisor[pin] = 0;
            resetArpState(pin, now);
            nextCap = getNextRepeatMicros(pin);
            catchupSteps++;
            continue;
          }
        }
      }
      nextCap = getNextRepeatMicros(pin);
      catchupSteps++;
    }
  }
}

float getRepeatSpeed(byte pin) {
  byte currentRepeatSpeedDivisor = usesLocalRepeatSettings(pin) ? pushRepeatSpeed[pin][1] : liveRepeatSpeedDivisor[pin];
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
  unsigned long repeats = pushElapsedRepeats[pin];
  unsigned long elapsedMicros = repeats * pushDuration;

  unsigned long nextTriggerTime = pushedTime[pin] + elapsedMicros;
  return nextTriggerTime;
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
    if (bpm != newBpm && (bpm > newBpm + 1 || bpm < newBpm - 1)) {
      byte previousBpm = bpm;
      bpm = newBpm;
      // displayPrintInt(bpm);
      oneNoteTime = getNoteMicros();
      if (newBpm != 0) {
        startTime = (startTime * previousBpm) / newBpm;
      }
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

  currentInputState = {false, false, false, false, false, false};
  for (byte position = 0; position < 4; position++) {
    SignalRef sw = switches[position].signal;
    byte mask = switches[position].mask;
    byte switchRaw = readDigitalSignalStable(sw);
    bool modeActive = isModeSwitchActive(switchRaw);
    if (modeActive) {
      if (mask == CC_MASK) {
        currentInputState.cc = true;
      }
      else if (mask == REPEAT_MASK) {
        currentInputState.repeat = true;
      }
      else if (mask == ULTRASONIC_MASK) {
        currentInputState.ultrasonic = true;
      }
      else if (mask == INIT_MASK) {
        currentInputState.init = true;
      }
    }
  }

  currentInputState.leftPush = isEncoderPushActive(readDigitalSignalStable(P1SW));
  leftPush = currentInputState.leftPush ? PUSHED : RELEASED;
  currentInputState.rightPush = isEncoderPushActive(readDigitalSignalStable(P2SW));
  rightPush = currentInputState.rightPush ? PUSHED : RELEASED;

  currentPlayMode = B00000000;
  if (currentInputState.cc) {
    currentPlayMode |= CC_MASK;
  }
  if (currentInputState.repeat) {
    currentPlayMode |= REPEAT_MASK;
  }
  if (currentInputState.ultrasonic) {
    currentPlayMode |= ULTRASONIC_MASK;
  }
  if (currentInputState.init) {
    currentPlayMode |= INIT_MASK;
  }
}

void updateMidiControlFromEncoder(byte selected) {
  if (encoderVal[selected] != encoderPos[selected]) {
    midiCC[selected] = getMidiValueFromEncoder(midiCC[selected], encoderVal[selected], encoderPos[selected]);
    encoderPos[selected] = encoderVal[selected];
    display_iface::clear();
    display_iface::print('C');
    displayPrint(midiCC[selected], false, false);
  }
}

bool sendControlChangeIfChanged(byte lane, byte ccNumber, byte ccValue) {
  if (lastSentCCNumber[lane] == ccNumber && lastSentCCValue[lane] == ccValue) {
    return false;
  }
  app_midi::sendControlChange(ccNumber, ccValue, midiChannel);
  lastSentCCNumber[lane] = ccNumber;
  lastSentCCValue[lane] = ccValue;
  return true;
}

bool shouldSuppressLiveDisplay() {
  return (leftPush == PUSHED || rightPush == PUSHED);
}

bool shouldSuppressLiveCCDisplay() {
  return shouldSuppressLiveDisplay();
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
  display_iface::clear();
  display_iface::print('c');
  displayPrint(midiCCValue[selected], false, false);
}

void updateCCValueFromFader(byte selected) {
  if (faderVal[selected] == faderPos[selected]) {
    return;
  }
  unsigned long nowUs = micros();
  if (FADER_CC_MIN_UPDATE_INTERVAL_US > 0 &&
      lastFaderCcSentMicros[selected] != 0 &&
      (unsigned long)(nowUs - lastFaderCcSentMicros[selected]) < FADER_CC_MIN_UPDATE_INTERVAL_US) {
    return;
  }
  faderPos[selected] = faderVal[selected];
  byte newCCValue = getMidiValueFromFader(selected);
  if (!sendControlChangeIfChanged(selected, midiCC[selected], newCCValue)) {
    return;
  }
  lastFaderCcSentMicros[selected] = nowUs;
  midiCCValue[selected] = newCCValue;
  if (shouldSuppressLiveCCDisplay()) {
    return;
  }
  display_iface::clear();
  display_iface::print('c');
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

byte getCurrentRepeatSpeedDivisor() {
  if (selectedPushPin != -1 && usesLocalRepeatSettings(selectedPushPin)) {
    return pushRepeatSpeed[selectedPushPin][1];
  }
  return repeatSpeedDivisor;
}

byte clampRepeatSpeedDivisor(int divisor) {
  if (divisor > 64) {
    return 64;
  }
  if (divisor < 1) {
    return 1;
  }
  return (byte) divisor;
}

byte stepRepeatSpeedDivisor(byte currentDivisor, int direction) {
  if (direction > 0) {
    for (byte i = 0; i < NB_REPEAT_SPEED_DIVISORS; i++) {
      if (REPEAT_SPEED_DIVISORS[i] > currentDivisor) {
        return REPEAT_SPEED_DIVISORS[i];
      }
    }
    return currentDivisor;
  }

  for (int i = NB_REPEAT_SPEED_DIVISORS - 1; i >= 0; i--) {
    if (REPEAT_SPEED_DIVISORS[i] < currentDivisor) {
      return REPEAT_SPEED_DIVISORS[i];
    }
  }
  return currentDivisor;
}

void displayRepeatSpeedDivisor(byte divisor) {
  display_iface::clear();
  display_iface::setColonOn(true);
  char label[6] = " 1";
  itoa(divisor, label + 2, 10);
  display_iface::print(label);
  display_iface::setColonOn(false);
}

void applyRepeatSpeedDivisor(byte divisor) {
  unsigned long now = micros();
  if (selectedPushPin != -1 && usesLocalRepeatSettings(selectedPushPin)) {
    pushRepeatSpeed[selectedPushPin][1] = divisor;
    liveRepeatSpeedDivisor[selectedPushPin] = divisor;
    pendingRepeatSpeedDivisor[selectedPushPin] = 0;
    pendingRepeatSpeedChange[selectedPushPin] = false;
    resetArpState(selectedPushPin, now);
  }
  else {
    repeatSpeedDivisor = divisor;
    for (byte pad = 0; pad < NB_PUSH; pad++) {
      if (usesLocalRepeatSettings(pad)) {
        continue;
      }
      liveRepeatSpeedDivisor[pad] = divisor;
      pendingRepeatSpeedDivisor[pad] = 0;
      pendingRepeatSpeedChange[pad] = false;
      resetArpState(pad, now);
    }
  }
  displayRepeatSpeedDivisor(divisor);
}

void updateArpRateFromEncoder(byte selected) {
  if (encoderVal[selected] == encoderPos[selected]) {
    return;
  }

  int delta = encoderVal[selected] - encoderPos[selected];
  int direction = delta > 0 ? 1 : -1;
  int steps = delta > 0 ? delta : -delta;
  byte nextDivisor = getCurrentRepeatSpeedDivisor();
  for (int i = 0; i < steps; i++) {
    nextDivisor = stepRepeatSpeedDivisor(nextDivisor, direction);
  }

  encoderPos[selected] = encoderVal[selected];
  if (nextDivisor != getCurrentRepeatSpeedDivisor()) {
    applyRepeatSpeedDivisor(nextDivisor);
  }
}

void updatePreciseArpRateFromEncoder(byte selected) {
  if (encoderVal[selected] == encoderPos[selected]) {
    return;
  }

  byte nextDivisor = clampRepeatSpeedDivisor(
    (int) getCurrentRepeatSpeedDivisor() + encoderVal[selected] - encoderPos[selected]
  );
  encoderPos[selected] = encoderVal[selected];
  if (nextDivisor != getCurrentRepeatSpeedDivisor()) {
    applyRepeatSpeedDivisor(nextDivisor);
  }
}

void updateBpmFromEncoder(byte selected) {
  if (encoderVal[selected] == encoderPos[selected]) {
    return;
  }

  int delta = encoderVal[selected] - encoderPos[selected];
  int nextBpm = (int) bpm + delta;
  if (nextBpm < 30) {
    nextBpm = 30;
  }
  if (nextBpm > 300) {
    nextBpm = 300;
  }

  encoderPos[selected] = encoderVal[selected];
  if ((byte) nextBpm == bpm) {
    return;
  }

  bpm = (byte) nextBpm;
  oneNoteTime = getNoteMicros();
  resetAllArpStates(micros());

  display_iface::clear();
  display_iface::setColonOn(false);
  char label[6];
  itoa(bpm, label, 10);
  strcat(label, "b");
  display_iface::print(label);
}

void updateArpStepNumberFromEncoder(byte selected) {
  if (encoderVal[selected] == encoderPos[selected]) {
    return;
  }

  int delta = encoderVal[selected] - encoderPos[selected];
  byte currentStep = repeatArpStepNumber;
  if (selectedPushPin != -1 && usesLocalRepeatSettings(selectedPushPin)) {
    currentStep = pushRepeatStepNumber[selectedPushPin];
  }
  int nextStep = (int) currentStep + delta;
  if (nextStep < 2) {
    nextStep = 2;
  }
  if (nextStep > 16) {
    nextStep = 16;
  }

  encoderPos[selected] = encoderVal[selected];
  if ((byte) nextStep == currentStep) {
    return;
  }

  if (selectedPushPin != -1 && usesLocalRepeatSettings(selectedPushPin)) {
    pushRepeatStepNumber[selectedPushPin] = (byte) nextStep;
  }
  else {
    repeatArpStepNumber = (byte) nextStep;
  }
  resetAllArpStates(micros());

  display_iface::clear();
  display_iface::setColonOn(false);
  char label[6] = "St";
  itoa(nextStep, label + 2, 10);
  display_iface::print(label);
}

void updateUltrasonicCC(byte selected) {
  if (encoderVal[selected] != encoderPos[selected]) {
    ultrasonicCC = getMidiValueFromEncoder(ultrasonicCC, encoderVal[selected], encoderPos[selected]);
    display_iface::clear();
    display_iface::print('C');
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

    display_iface::clear();
    display_iface::print('d');
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
    app_midi::sendControlChange(ultrasonicCC, ultrasonicControlValue, midiChannel);
    if (!shouldSuppressLiveCCDisplay()) {
      display_iface::clear();
      display_iface::print('c');
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
    display_iface::clear();
    display_iface::setColonOn(false);
    char label[5];
    label[0] = 'C';
    label[1] = '0' + presetCC / 100;
    label[2] = '0' + (presetCC / 10) % 10;
    label[3] = '0' + presetCC % 10;
    label[4] = '\0';
    display_iface::print(label);
  }
}

void displayPrint(const char string[], bool semicolon, bool clear) {
  prepareDisplay(semicolon, clear);
  display_iface::print(string);
}

void displayPrint(int string, bool semicolon, bool clear) {
  prepareDisplay(semicolon, clear);
  char label[12];
  itoa(string, label, 10);
  display_iface::print(label);
}

void displayPrintString(const char s[]) {
  display_iface::setColonOn(false);
  display_iface::clear();
  display_iface::print(s);
}

void prepareDisplay(bool semicolon, bool clear) {
  semicolon ? display_iface::setColonOn(true) : display_iface::setColonOn(false);
  if (clear) {
    display_iface::clear();
  }
}

void sendNote(byte note, byte velocity, bool on) {
  for (
    byte offset = 0;
    offset < MAX_NOTES;
    offset++
  ) {
    byte interval = readChordInterval(selectedChord, offset);
    if (interval == UNASSIGNED) {
      // End of chord definition; do not leave function so magnet state can be updated.
      break;
    }
    if ((note + interval) >= 128) {
      // Skip invalid note but keep processing and update outputs below.
      continue;
    }
    on ?
      app_midi::sendNoteOn(note + interval, velocity, midiChannel) :
      app_midi::sendNoteOff(note + interval, velocity, midiChannel);
  }

  if (on) {
    io_iface::writeDigital(LED_BUILTIN, HIGH);
  }
  else {
    io_iface::writeDigital(LED_BUILTIN, LOW);
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
