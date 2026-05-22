# MIDI P0AH PLAY (proto-serial)

DIY performance controller firmware (Arduino sketch) that outputs **MIDI over Serial**. It features a scale/chord engine, arp performance controls, CC control lanes, ultrasonic "D-Beam," pad lock/latch workflows, and MIDI clock sync.

### MIDI Signals Summary
* **Note On/Off:** Pads send note events (single notes or chord voicings depending on mode/scale settings).
* **Control Change (CC):** Encoders, faders, pad presets, and the ultrasonic sensor send CC values.
* **Clock/Transport Sync:** Listens to MIDI Clock, Start, Stop, Continue, and Song Position Pointer for tempo-synced arp behavior.

![MIDI P0AH PLAY](images/1776264780911.jpg)

## ✅ Current Firmware Feature Set

### Musical Engine
* 8 pads (`P1..P8`) mapped to semitone positions with optional scale remap.
* Global velocity + per-pad velocity lock.
* Global base note / transpose editing in Init workflow.
* Global octave control (fader + encoder), with note-layout updates applied live.
* Chord engine with **14 chord types**:
  * `NOTE`, `MAJ`, `MIN`, `AUG`, `dIM`, `SUS2`, `SUS4`, `7th`, `MAJ7`, `MIN7`, `d7`, `5th`, `Ad9`, `m7b6`
* Scale engine with **10 scale layouts**:
  * `SEMI`, `MAJ`, `MIN_`, `BLUE`, `BLU_`, `PENT`, `DOR `, `JAPN`, `DRUM`, `MIX `
* Chord, scale, and arp selectors wrap around instead of stopping at the ends.

### Arp / Gate
* `SW_REPEAT` is now an **arp mode** rather than a simple note-repeat mode.
* Arp playback uses the current 8-pad scale layout as its note pool and chord-voices each step through the selected chord.
* Tempo/gate behavior is per pad, with scheduled NoteOff handling to avoid stuck or overly long repeated notes.
* Pad latch/lock is available inside arp mode, so a pad can continue running after release.
* Current arp types:

| Label | Meaning |
| :--- | :--- |
| `NOtE` | Retrigger the current pad root/chord only |
| `UP` | Walk upward through the valid pad-layout slots |
| `dn` | Walk downward through the valid pad-layout slots |
| `dU` | Bounce downward first, without repeating endpoints |
| `Ud` | Bounce upward first, without repeating endpoints |
| `rAnd` | Uniform random slot selection, repeats allowed |
| `rnNo` | Random slot selection without immediate repeats |
| `SHFL` | Shuffle valid slots, play the cycle, then reshuffle |
| `CNVr` | Converge from the outside toward the center |
| `Ordr` | Use pad press order, anchored by the first active pressed/latched pad |
| `ASGN` | Stable as-played pad order |

* Ordered arp modes (`Ordr`, `ASGN`) treat the **first active pressed/latched pad** as the anchor lane. Later pads extend the note pool instead of starting overlapping arp sequences.

### CC Engine
* 2 editable CC lanes (`midiCC[0]`, `midiCC[1]`) with values from encoders/faders.
* Pad CC presets (in CC mode):
  * `P1→CC91`, `P2→CC92`, `P3→CC93`, `P4→CC94`,
  * `P5→CC95`, `P6→CC98`, `P7→CC99`, `P8→CC100`
* CC preset display format on 4-digit screen:
  * `C091 ... C100`

### Sync / Transport / Safety
* MIDI realtime handling: `CLOCK`, `START`, `STOP`, `CONTINUE`, `SPP`.
* Incoming MIDI clock drives arp timing when transport sync is active.
* During synced arp playback, note-layout changes (for example octave, chord, scale, or lock-related changes) take effect on the next scheduled step instead of forcing an immediate base-note restart.
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

## 🎹 Default Pin & Multiplexer Mapping (Nano Profile)

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

## 🧩 Compile-Time Wiring and Board Profiles

Firmware wiring and board selection is now profile-driven. You can switch targets without editing core logic.

### Built-in profiles

* **Board profiles**
  * `BOARD_NANO`
  * `BOARD_PRO_MICRO`
* **Wiring profile headers**
  * `"wirings/nano_default.h"`
  * `"wirings/pro_micro_clone_safe.h"`
* **Mapping profiles**
  * `MAPPING_DEFAULT`

Profile definitions live in:

* `src/profiles/boards/`
* `src/profiles/wirings/`
* `src/profiles/mappings/`
* Selection/validation: `src/profiles/selected_profiles.h`
* Global selectors: `src/config/build_config.h`

### Existing profile selection (tracked)

Set profile selectors in `platformio.ini` via `build_flags`.

`nanoatmega328` uses serial Nano defaults:

* `APP_BOARD_PROFILE=BOARD_NANO`
* `APP_WIRING_PROFILE_HEADER="wirings/nano_default.h"`
* `APP_MAPPING_PROFILE=MAPPING_DEFAULT`
* `APP_MIDI_TRANSPORT=MIDI_TRANSPORT_SERIAL`

`promicro16` uses a Pro Micro clone-safe wiring baseline:

* `APP_BOARD_PROFILE=BOARD_PRO_MICRO`
* `APP_WIRING_PROFILE_HEADER="wirings/pro_micro_clone_safe.h"`
* `APP_MAPPING_PROFILE=MAPPING_DEFAULT`
* `APP_MIDI_TRANSPORT=MIDI_TRANSPORT_SERIAL`

### Local custom profiles (not versioned)

Use local files when you want custom mappings/wiring without changing tracked repository defaults.

1. Create local build override:
   * `src/config/build_config.local.h`
   * Starter template: `src/config/build_config.local.h.example`
2. (Optional) Create local profile definitions:
   * `src/profiles/local/local_profiles.h`
   * Starter template: `src/profiles/local/local_profiles.h.example`
   * Local wiring supports encoder pins, push button pins (`P1..P8`), faders, LEDs, ultrasonic, magnet, and mux switch channels (`SW_CC`, `SW_REPEAT`, `SW_ULTRASONIC`, `SW_PLAY`).
3. Switch selectors in `build_config.local.h`.

These local paths are ignored by git:

* `src/config/build_config.local.h`
* `src/profiles/local/`

Minimal example (`src/config/build_config.local.h`):

```cpp
#ifndef BUILD_CONFIG_LOCAL_H
#define BUILD_CONFIG_LOCAL_H

#define APP_BOARD_PROFILE BOARD_NANO
#define APP_WIRING_PROFILE_HEADER "local/local_profiles.h"
#define APP_MAPPING_PROFILE MAPPING_LOCAL
#define APP_MIDI_TRANSPORT MIDI_TRANSPORT_SERIAL

#endif
```

Quick start:

* `cp src/config/build_config.local.h.example src/config/build_config.local.h`

If `APP_MAPPING_PROFILE` is `MAPPING_LOCAL`, firmware expects:

* `src/profiles/local/local_profiles.h`
* `kLocalMappingProfile` definition compatible with `MappingProfile` (pad CC presets + default CC lane numbers/values + default ultrasonic CC + fader calibration + ultrasonic tuning).
* Chord and scale tables are built into `powaplay.ino` and are not part of local mapping overrides.
* Fader/ultrasonic tuning fields include min/max/threshold and smoothing/deadband/update interval parameters.

If local mapping is selected and missing, compilation fails with an explicit error.

If `APP_WIRING_PROFILE_HEADER` points to a missing header, compilation fails with an explicit include error.

---

## 🕹 Operating Modes

Toggle the physical Mode Switches to change functionality:

* **Standard Mode:**
  * Fader 1 = Velocity, Fader 2 = Octave.
  * Encoder 1 = Velocity, Encoder 2 = Octave.
  * Right encoder push: scale select.
  * Left encoder push: chord select.
  * Chord and scale selectors wrap around.
  * Pad lock/unlock available via encoder-push + pad workflows.

* **CC Mode (`SW_CC`)**
  * Faders edit CC values for lanes 1 and 2.
  * With **no encoder push held**, encoder 1 and encoder 2 edit the CC values for lanes 1 and 2.
  * Hold **Left encoder push** (`P1SW`) to edit/select **CC lane 2 number** with encoder 2 and select pad-based CC presets for lane 1.
  * Hold **Right encoder push** (`P2SW`) to edit/select **CC lane 1 number** with encoder 1 and select pad-based CC presets for lane 2.
  * Preset CC labels are displayed as `C091..C100`.

* **Arp Mode (`SW_REPEAT`)**
  * Left encoder push: arp rate / divisor edit.
  * Right encoder push: arp type select.
  * Arp selector wraps around.
  * Pad arp lock/unlock workflows are available in arp context.

* **Ultrasonic Mode (`SW_ULTRASONIC`)**
  * Hold **Right encoder push** to set ultrasonic target CC number.
  * Left encoder push + turn encoder 1: set max ultrasonic distance range.
  * Sensor sends continuous CC based on measured distance.

* **DRUM scale note path**
  * The `DRUM` scale uses a direct GM drum-note map (`36, 38, 42, 46, 41, 45, 49, 51`) for Melodics/E-Drums compatibility.
  * In DRUM scale, this direct map is used for playback instead of the generic scale-step transpose path.

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
2. Build firmware (Nano baseline):
   * `python -m platformio run -e nanoatmega328`
3. Upload firmware (Nano baseline):
   * `python -m platformio run -e nanoatmega328 -t upload --upload-port <PORT>`
4. Build alternate tracked profile (Pro Micro clone-safe):
   * `python -m platformio run -e promicro16`
5. Open serial monitor (firmware baud rate):
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
