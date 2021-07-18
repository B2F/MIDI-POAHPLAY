/**
 * MIDI P0Ah PLAy
 */

#include "MIDI.h"
#include "TM1637.h"
#include <RotaryEncoder.h>
#include <light_CD74HC4067.h>

const long BAUD_RATE = 57600;

// Encoders:

const int NB_ENCODERS = 2;

const int P1CLK = 4;
const int P1DT = 3;
const int P1SW = 14;
const int P2CLK = 6;
const int P2DT = 5;
const int P2SW = 15;

// RotaryEncoder P1(P1CLK, P1DT, RotaryEncoder::LatchMode::TWO03);
// RotaryEncoder P2(P2CLK, P2DT, RotaryEncoder::LatchMode::TWO03);

// RotaryEncoder encoder[NB_ENCODERS] = {P1, P2};
// int encoderSwitches[NB_ENCODERS] = {P1SW, P2SW};
int encoderPos[NB_ENCODERS] = {1, 1};
int encoderState[NB_ENCODERS] = {0, 0};

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
int globalStartNote = 60;

// LCD

const int LCD_CLK = 12;
const int LCD_DIO = 11;

TM1637 tm(LCD_CLK, LCD_DIO);

// Push buttons:

const int PUSHED = LOW;
const int RELEASED = HIGH;
const int NB_PUSH = 8;

int pushPin[NB_PUSH] = {4, 3, 2, 5, 6, 7, 8, 9};
int pushNote[NB_PUSH] = {60, 62, 63, 64, 65, 67, 71, 72};
int pushVelocity[NB_PUSH] = {127, 127, 127, 127, 127, 127, 127, 127};
int isPushed[NB_PUSH] = {RELEASED, RELEASED, RELEASED, RELEASED, RELEASED, RELEASED, RELEASED, RELEASED};

// Faders:

const int F1 = A6;
const int F2 = A7;

// Leds:

const int L1 = 10;
const int L2 = 9;
const int L3 = 8;
const int L4 = 7;

void setup() {

  Serial.begin(BAUD_RATE);

  tm.begin();

  // tm.display("PLAY");
  tm.clearScreen();

  pinMode(P1CLK, INPUT);
  pinMode(P1DT, INPUT);
  pinMode(P2CLK, INPUT);
  pinMode(P2DT, INPUT);

  // Leds:
  pinMode(L1, OUTPUT);
  pinMode(L2, OUTPUT);
  pinMode(L3, OUTPUT);
  pinMode(L4, OUTPUT);

  // mux sig:
  pinMode(A0, INPUT_PULLUP);

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
}

void loop() {

  // for (int e = 0; e < NB_ENCODERS; e++) {
  //   encoder[e].tick();
  //   int position = encoder[e].getPosition();
  //   if (0 != encoderPos[e] && encoderPos[e] != position) {
  //     encoderPos[e] = position;
  //     String positionString = String(position);
  //     tm.display(positionString);
  //   }
  //   Serial.println(encoderPos[e]);

  //   int state = digitalRead(encoderSwitches[e]);
  //   if (state != encoderState[e]) {
  //       encoderState[e] = state;
  //       Serial.println("Changed");
  //       digitalWrite(LED_BUILTIN, LOW);
  //       tm.clearScreen();
  //   }
  // }

  for (int p = 0; p < NB_PUSH; p++) {

    mux.channel(pushPin[p]);
    int sensorVal = digitalRead(MUXSIG);

    if (sensorVal == PUSHED && isPushed[p] == RELEASED) {
        MIDI.sendNoteOn(pushNote[p], pushVelocity[p], midiChannel);
        isPushed[p] = PUSHED;
        digitalWrite(LED_BUILTIN, HIGH);
    }
    if (sensorVal == RELEASED && isPushed[p] == PUSHED) {
        isPushed[p] = RELEASED;
        MIDI.sendNoteOff(pushNote[p], pushVelocity[p], midiChannel);
        digitalWrite(LED_BUILTIN, LOW);
    }
    delay(5);
  }

  digitalWrite(L1, HIGH);
  // digitalWrite(L2, HIGH);
  digitalWrite(L3, HIGH);
  // digitalWrite(L4, HIGH);

  delay(10);
}
