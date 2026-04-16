# MIDI P0AH PLAY (proto-serial)

DIY performance controller firmware (Arduino sketch) that outputs **MIDI over Serial**. It features a scale/chord engine, ultrasonic "D-Beam," and a pad-locking system.

## 🛠 Required Hardware

This project is designed for a specific hardware footprint. Because it uses **8 pads, 4 mode switches, and 2 encoder buttons**, a multiplexer is mandatory.

![MIDI P0AH PLAY](images/1776264780911.jpg)

### MIDI Signals Summary
* **Note On/Off:** Pads send note events (single notes or chord voicings depending on mode/scale settings).
* **Control Change (CC):** Encoders, faders, pad presets, and the ultrasonic sensor send CC values.
* **Clock/Transport Sync:** Listens to MIDI Clock, Start, Stop, Continue, and Song Position Pointer for tempo-synced repeat behavior.

### 1. Microcontroller
*   **Recommended:** **Arduino Nano** or **Pro Mini** (ATmega328P).
*   **Note:** This sketch uses pins **A6 and A7** for the faders. These pins are physically present on the Nano and Pro Mini but **do not exist on the Arduino Uno**. If using an Uno, you must rewire faders to A4/A5 and move the Multiplexer pins.

### 2. Multiplexer
*   **Model:** **CD74HC4067** (16-Channel Analog/Digital Multiplexer).
*   This is the "brain" of the inputs, handling all 8 pads, the 4 mode switches, and the 2 encoder push-buttons via a single analog pin (`A0`).

### 3. Other Components
*   **Rotary Encoders:** 2x Standard KY-040 (or similar).
*   **Display:** 4-digit TM1637 7-segment display.
*   **Sensor:** HC-SR04 Ultrasonic Sensor.
*   **Physical Feedback:** 4x LEDs (Tempo) and 1x Solenoid/Magnet (Triggered via Pin A5).

## 🎹 Pin & Multiplexer Mapping

| Component | Pin / Mux Channel |
| :--- | :--- |
| **Mux SIG** | **A0** (The entry point for 14 inputs) |
| **Mux Address** | **A1, A2, A3, A4** (S0-S3) |
| **Fader 1 & 2** | **A6, A7** (Dedicated Analog) |
| **Encoders** | D2/D4 (Enc 1) and D3/D5 (Enc 2) |
| **Display** | D12 (CLK), D11 (DIO) |
| **Ultrasonic** | D13 (Trig), D6 (Echo) |
| **Magnet Out** | **A5** (Active on Note On) |

### Multiplexer (CD74HC4067) Channel Map:
| Channel | Function |
| :--- | :--- |
| **0** | `SW_CC` (Mode Switch) |
| **1** | `SW_REPEAT` (Mode Switch) |
| **2 – 9** | **Pads 1 – 8** (Performance Buttons) |
| **10** | `SW_ULTRASONIC` (Mode Switch) |
| **11** | `SW_PLAY` (Init/Reset Mode Switch) |
| **14** | `P2SW` (Right Encoder Push) |
| **15** | `P1SW` (Left Encoder Push) |

---

## 🕹 Operating Modes

Toggle the physical Mode Switches to change functionality:

*   **Standard Mode (All OFF):** Faders control Velocity/Octave. Hold Encoder switches to change **Scales** (`SEMI`, `MAJ`, `MIN`, `BLUE`) or **Chords** (`MAJ`, `MIN`, `AUG`).
*   **CC Mode (`SW_CC` ON):** Encoders/Faders send MIDI CCs. Use pads as CC presets.
*   **Repeat Mode (`SW_REPEAT` ON):** Arpeggiator mode. Hold **Left Encoder Switch** to change speed. Lock repeats by holding Pad + Left Switch.
*   **Ultrasonic Mode (`SW_ULTRASONIC` ON):** Distance-based CC control (D-Beam style).
*   **Init Mode (`SW_PLAY` ON):** Set MIDI Channel and Base Note. Hold **Left Encoder Switch** while toggling this off to perform a **Factory Reset**.


---

## 💻 Software Setup & MIDI Routing

Since this device communicates over a standard Serial port (not Native USB MIDI), you must bridge the signal on your computer.

### The Windows Workflow (loopMIDI + Hairless)
On Windows, MIDI software cannot "see" Serial data directly. You need two tools:
1.  **[loopMIDI](https://www.tobias-erichsen.de/software/loopmidi.html):** Create a virtual MIDI port (e.g., call it "P0Ah Port").
2.  **[Hairless MIDI <-> Serial Bridge](https://projectgus.github.io/hairless-midiserial/):**
    *   Set **Serial Port** to your Arduino's COM port.
    *   Set **MIDI Out** to your loopMIDI virtual port.
    *   Set **MIDI In** to your loopMIDI virtual port (for clock sync).
    *   Go to `Preferences` and set the **Baud Rate to 38400**.
3.  **In your DAW:** Select the virtual (LoopMidi) Port as your MIDI Input and Output (to send clock).

### Alternative bridge solution
For modern WSL compatible serial MIDI bridge, use: **[SerialMidi-WMS](https://github.com/B2F/SerialMidi-WMS)**

---

## ⚠️ Troubleshooting
*   **Baud Rate:** If you see "Garbage" in Hairless MIDI, ensure the baud rate is exactly **38400**.
*   **Ghost Triggers:** Ensure the CD74HC4067 `EN` (Enable) pin is connected to **Ground**.
*   **Uno Users:** If you are forced to use an Uno, change `F1` and `F2` in the code to `A2` and `A3`, and move your Multiplexer Address pins to the Digital rail.
