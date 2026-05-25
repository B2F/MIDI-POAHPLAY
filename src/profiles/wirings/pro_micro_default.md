## pro_micro_default wiring map

Topology: `SingleMuxWiringProfile` (MUX1 only)

| Signal | Source | Value | Direction | Notes |
|---|---|---:|---|---|
| encoder1Clk | PIN | 2 | input | Encoder 1 quadrature A |
| encoder1Dt | PIN | 4 | input | Encoder 1 quadrature B |
| encoder1Sw | MUX1 | 15 | input | Encoder 1 push |
| encoder2Clk | PIN | 3 | input | Encoder 2 quadrature A |
| encoder2Dt | PIN | 5 | input | Encoder 2 quadrature B |
| encoder2Sw | MUX1 | 14 | input | Encoder 2 push |
| mux1Sig | PIN | A0 | input | CD74HC4067 #1 SIG |
| mux1S0 | PIN | A1 | output | CD74HC4067 #1 address S0 |
| mux1S1 | PIN | A2 | output | CD74HC4067 #1 address S1 |
| mux1S2 | PIN | A3 | output | CD74HC4067 #1 address S2 |
| mux1S3 | PIN | A4 | output | CD74HC4067 #1 address S3 |
| lcdClk | PIN | 12 | output | TM1637 CLK |
| lcdDio | PIN | 11 | output | TM1637 DIO |
| fader1 | PIN | A2 | input | Analog fader 1 |
| fader2 | PIN | A3 | input | Analog fader 2 |
| led1 | PIN | 10 | output | Pad column LED 1 |
| led2 | PIN | 9 | output | Pad column LED 2 |
| led3 | PIN | 8 | output | Pad column LED 3 |
| led4 | PIN | 7 | output | Pad column LED 4 |
| triggerPin | PIN | 13 | output | Ultrasonic trigger |
| echoPin | PIN | 6 | input | Ultrasonic echo |
| magnetPin | PIN | A5 | output | Solenoid/magnet output |
| swCcChannel | MUX1 | 0 | input | Mode switch CC |
| swRepeatChannel | MUX1 | 1 | input | Mode switch REPEAT |
| swUltrasonicChannel | MUX1 | 10 | input | Mode switch ULTRASONIC |
| swPlayChannel | MUX1 | 11 | input | Mode switch PLAY |
| pushPins[0] | MUX1 | 4 | input | Pad P1 |
| pushPins[1] | MUX1 | 3 | input | Pad P2 |
| pushPins[2] | MUX1 | 2 | input | Pad P3 |
| pushPins[3] | MUX1 | 5 | input | Pad P4 |
| pushPins[4] | MUX1 | 6 | input | Pad P5 |
| pushPins[5] | MUX1 | 7 | input | Pad P6 |
| pushPins[6] | MUX1 | 8 | input | Pad P7 |
| pushPins[7] | MUX1 | 9 | input | Pad P8 |
