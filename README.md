# MIDI P0AH PLAY (proto-serial)

DIY performance controller firmware (Arduino sketch) that outputs **MIDI over Serial**. It features a scale/chord engine, CC performance controls, ultrasonic "D-Beam," repeat/gate behavior, pad lock/unlock, and MIDI clock sync.

### MIDI Signals Summary
* **Note On/Off:** Pads send note events (single notes or chord voicings depending on mode/scale settings).
* **Control Change (CC):** Encoders, faders, pad presets, and the ultrasonic sensor send CC values.
* **Clock/Transport Sync:** Listens to MIDI Clock, Start, Stop, Continue, and Song Position Pointer for tempo-synced repeat behavior.

![MIDI P0AH PLAY](images/1776264780911.jpg)

## ✅ Current Firmware Feature Set

### Notes / Musical Engine
* 8 pads (`P1..P8`) mapped to semitone positions with optional scale remap.
* Global velocity + per-pad velocity lock.
* Global base note / transpose editing in Init workflow.
* Global octave control (fader + encoder), with live retrigger of held pads.
* Chord engine with **10 chord types**:
  * `NOTE`, `MAJ`, `MIN`, `AUG`, `dIM`, `SUS2`, `SUS4`, `7th`, `MAJ7`, `MIN7`
* Scales currently available:
  * `SEMI`, `MAJ`, `MIN_`, `BLUE`, `BLU_`

### CC Engine
* 2 editable CC lanes (`midiCC[0]`, `midiCC[1]`) with values from encoders/faders.
* Pad CC presets (in CC mode):
  * `P1→CC20`, `P2→CC21`, `P3→CC22`, `P4→CC23`,
  * `P5→CC24`, `P6→CC25`, `P7→CC26`, `P8→CC27`
* CC preset display format on 4-digit screen:
  * `C020 ... C027`

### Repeat / Gate
* Repeat clocking per pad, with lock/unlock behavior.
* Repeat divisor editing.
* Scheduled NoteOff gate to avoid stuck/overlapping repeat notes.

### Sync / Transport / Safety
* MIDI realtime handling: `CLOCK`, `START`, `STOP`, `CONTINUE`, `SPP`.
* Tick-based tempo LED display (one LED at a time).
* Panic on Init-switch edge (`All Notes Off` + `All Sound Off`) and local note state cleanup.

### LED Behavior
* 4 LEDs reflect pad columns when playing pads:
  * LED1 = pads 1+5
  * LED2 = pads 2+6
  * LED3 = pads 3+7
  * LED4 = pads 4+8
* During MIDI sync and no held pads: LEDs show beat phase (single LED chase).

## 🛠 Required Hardware

This project is designed for a specific hardware footprint. Because it uses **8 pads, 4 mode switches, and 2 encoder buttons**, a multiplexer is mandatory.

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

* **Standard Mode:**
  * Fader 1 = Velocity, Fader 2 = Octave.
  * Encoder 1 = Velocity, Encoder 2 = Octave.
  * Right encoder push: scale select.
  * Left encoder push: chord select.
  * Pad lock/unlock available via encoder-push + pad workflows.

* **CC Mode (`SW_CC`)**
  * Faders + (released) encoders edit CC values.
  * **CC number selection with encoder push:**
    * Hold **Left encoder push** (`P1SW`) to edit/select **CC lane 1**.
      * Turning encoder 1 changes the CC number for lane 1.
      * Pressing pads selects preset CC numbers for lane 1 (`P1..P8 => CC20..CC27`).
    * Hold **Right encoder push** (`P2SW`) to edit/select **CC lane 2**.
      * Turning encoder 2 changes the CC number for lane 2.
      * Pressing pads selects preset CC numbers for lane 2 (`P1..P8 => CC20..CC27`).
  * Preset CC labels are displayed as `C020..C027`.

* **Repeat Mode (`SW_REPEAT`)**
  * Left encoder push: repeat speed/divisor edit.
  * Pad repeat lock/unlock workflows available while in repeat context.

* **Ultrasonic Mode (`SW_ULTRASONIC`)**
  * Right encoder push: set ultrasonic target CC number.
  * Left encoder push: set max ultrasonic distance range.
  * Sensor sends continuous CC based on measured distance.

* **Init / Setup Mode (`SW_PLAY` logic)**
  * Encoder 1: MIDI channel (`CHx`).
  * Encoder 2: base note / transpose reference (note name shown).
  * Left encoder push on init entry can trigger `reinit()` (factory-style reset of runtime settings).


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

### Firmware Flashing (PlatformIO)
Use PlatformIO CLI from the repository root:

1. Install PlatformIO Core (if needed):
   * `python -m pip install -U platformio`
2. Build firmware:
   * `python -m platformio run -e nanoatmega328`
3. Upload firmware:
   * `python -m platformio run -e nanoatmega328 -t upload --upload-port <PORT>`
4. Open serial monitor (firmware baud rate):
   * `python -m platformio device monitor -p <PORT> -b 38400`

Notes:
* `-t upload` compiles automatically before uploading.
* Replace `<PORT>` with your serial port (for example `COM3` on Windows or `/dev/ttyUSB0` on Linux).
* If needed, list available ports with `python -m platformio device list`.
* If upload fails because the port is busy, close serial monitor/bridge apps and retry.

---

## ⚠️ Troubleshooting
*   **Baud Rate:** If you see "Garbage" in Hairless MIDI, ensure the baud rate is exactly **38400**.
*   **Ghost Triggers:** Ensure the CD74HC4067 `EN` (Enable) pin is connected to **Ground**.
*   **Uno Users:** If you are forced to use an Uno, change `F1` and `F2` in the code to `A2` and `A3`, and move your Multiplexer Address pins to the Digital rail.
