# Desktop AI Companion (ESP32-S3 + Gemini) — Plan

## Context
Build a standalone desktop voice assistant. The ESP32-S3 listens for a wake word, streams mic audio directly to the Gemini Live API over WiFi, and plays the spoken reply. Choices: ESP32 direct (no laptop relay), wake word trigger, no servo for now, PlatformIO/Arduino firmware. The project directory is currently empty.

## Parts used / not used
- Used: ESP32-S3-N16R8 (8MB PSRAM is required for TLS + audio buffers), INMP441 mic, MAX98357 amp + 3W 4Ω speaker, breadboard (now the Makeronics BB-102), one push button (mute/reset fallback), jumper wires.
- Optional later: TP4056 + 18650 (battery), MG90S servo, TTP223, N20 motor.
- Not needed: N20 motor. Servo is deferred.
- Battery caveat: the TP4056 has no boost regulator. 3.7V into the dev board's 5V pin will brown out the LDO. Develop on USB power. For battery use, add a 5V boost converter (not in your list).

## Wiring (ESP32-S3, separate I2S buses)
- INMP441 (I2S0, RX): SCK→GPIO4, WS→GPIO5, SD→GPIO6, L/R→GND, VDD→3V3, GND→GND. (See the one-sided wiring update at the end.)
- MAX98357 (I2S1, TX): BCLK→GPIO15, LRC→GPIO7, DIN→GPIO16, VIN→5V, GND→GND, SD tied to VIN (always on, or a GPIO for mute). Speaker to its +/- outputs.
- Button: GPIO9 to GND (INPUT_PULLUP).
- Avoid GPIO 35–37 on the N16R8 (octal PSRAM). Pin numbers are adjustable in `config.h`.

## Architecture
1. **Wake word task** (core 1): INMP441 at 16 kHz mono 16-bit → ESP-SR WakeNet (`esp-sr` component). Built-in models are limited (e.g. "Hi ESP"). A custom phrase needs Espressif training or an Edge Impulse keyword model. Start with a built-in word, swap later.
2. **Session task** (core 0): on wake, open a TLS WebSocket to the Gemini Live API (`BidiGenerateContent`, model e.g. `gemini-live-*` native-audio). Send the setup message (system prompt, voice, audio response modality). Stream mic audio as base64 PCM 16 kHz `realtimeInput` chunks (~100 ms). Receive `serverContent` audio (PCM 24 kHz), decode base64, and push into a PSRAM ring buffer.
3. **Playback task**: ring buffer → I2S1 → MAX98357. Resample or set the I2S rate to 24 kHz.
4. **State machine**: IDLE (wake listening) → LISTENING → THINKING → SPEAKING → IDLE. Return to IDLE after `turnComplete` or on a silence/timeout. Mute the mic (or ignore wake word) during SPEAKING to avoid echo. Add a button to cancel.
5. **Secrets**: WiFi + API key in `secrets.h` (gitignored). Note: the key lives on the device, so use a restricted, quota-limited key. Ephemeral tokens are the safer option if later needed.

## Files (PlatformIO project in the working dir)
- `platformio.ini` — `esp32-s3-devkitc-1`, `board_build.arduino.memory_type = qio_opi`, PSRAM flags, 16MB flash partition table, libs: `links2004/WebSockets`, `bblanchon/ArduinoJson`, esp-sr (via ESP-IDF component / pioarduino platform).
- `src/main.cpp` — setup, task creation, state machine.
- `src/audio_in.{h,cpp}` — INMP441 I2S RX.
- `src/audio_out.{h,cpp}` — MAX98357 I2S TX + ring buffer.
- `src/wakeword.{h,cpp}` — WakeNet wrapper.
- `src/gemini_live.{h,cpp}` — WebSocket client, setup/send/receive, base64.
- `include/config.h`, `include/secrets.example.h`.

## Milestones (each independently verifiable)
1. Blink + serial + PSRAM check (`ESP.getPsramSize()` ≈ 8MB).
2. Mic test: record 5 s, print RMS levels / dump to serial and verify in Audacity.
3. Speaker test: play a sine tone and a stored PCM clip.
4. Gemini Live round trip using button trigger (no wake word): speak, hear the reply.
5. Add wake word; add state machine, echo handling, timeouts, reconnect logic.
6. Polish: LED/status, volume, system prompt/persona; later servo and battery.

## Verification
- Each milestone's serial output as above; end-to-end: say wake word → ask "what's the capital of France?" → audible spoken answer within ~2 s; confirm it returns to IDLE and works repeatedly for 10+ turns without heap exhaustion (log `ESP.getFreeHeap()` / free PSRAM per turn).

## Risks
- ESP32 TLS WebSocket + base64 audio is memory- and CPU-heavy: keep buffers in PSRAM, use 16 kHz chunks.
- Speaker echo into the mic: keep the mic away from the speaker; mute the mic while speaking.
- API/model names change: confirm the current Live API model ID and message schema in Google's docs at implementation time.

## Update: robot face (added later) — SUPERSEDED by the big colour screen (see the end)
- Part: 0.96" SSD1306 OLED, 128x64, 4-pin I2C (pins GND VDD SCL SDA), yellow/blue or white/blue two-colour: top 16 rows are one colour, the rest blue.
- Wiring: GND→GND, VDD→3V3, SCL→GPIO17, SDA→GPIO18 (I2C address 0x3C). Pins/address in `include/config.h`.
- Firmware: `src/face.{h,cpp}` runs a 20 fps animation task (moods: idle, happy, listen, think, speak, sleep); `main.cpp` sets the mood from session state. Optional: `FACE_ENABLED 0`, and it skips itself if no OLED is found.
- Kid-first milestone: `pio run -e face_demo -t upload` builds `src/face_demo.cpp`, which needs only ESP32 + OLED (no mic, speaker or WiFi). Cycles through every mood.
- Libraries: Adafruit SSD1306 + Adafruit GFX. Not yet compiled or run on hardware.
- The face uses the top 16-pixel band for a status word (READY, LISTENING, ...) and draws the eyes below it.

## Update: SUPERSEDED idea, pins split across both sides of the ESP32
- This was dropped. The author's ESP32-S3 board is ~1.0" wide: its pin rows sit in breadboard rows B and J, so only row A (top) is free and the bottom row of pins cannot be wired at all.

## Update: one-sided wiring (current design, verified against the author's board)
- Only the TOP row of pins is used. With the USB ports on the LEFT it reads: `G, 5V, 14, 13, 12, 11, 10, 9, 46, 3, 8, 18, 17, 16, 15, 7, 6, 5, 4, RST, 3V3, 3V3` (the board's other header, with GND/TX/RX/1/2/42..., is the unreachable bottom row). The 3V3/5V positions come from the usual DevKitC-1 layout and are unverified on the real board.
- Pins: mic SCK=4, WS=5, SD=6; amp BCLK=15, LRC=7, DIN=16; OLED SCL=17, SDA=18; button=9. All parts sit upright on the top half; `FACE_FLIP` is 0.
- Rails: top red = 3.3V (mic, OLED), top blue = GND. The amp's VIN comes straight from the 5V pin by a long wire.
- Pins were chosen so the nine lane wires nest with only 2 crossings (the 5V wire over the two OLED wires).

## Update: Arduino IDE sketches for kids
- `arduino/AI_Companion` (full robot) and `arduino/Face_Demo` (OLED only) are generated from `src/` + `include/` by `sh tools/make_arduino_sketches.sh`. Re-run it after any code change.
- Kid guide uses Arduino IDE 2 (board package esp32 3.x; libs WebSockets, Adafruit SSD1306, Adafruit GFX). Tools: ESP32S3 Dev Module, USB CDC On Boot Disabled (COM port), 16MB flash, OPI PSRAM, 16M partition.
- PlatformIO env no longer forces USB CDC on boot (serial on the COM port). Not compiled or run yet.

## Update: wake word ("Say its name") — implemented, untested
- `src/wakeword.{h,cpp}` wraps the Arduino `ESP_SR` library (WakeNet, phrase "Hi ESP"). `WAKEWORD_ENABLED` in `config.h` (default 0); `SESSION_IDLE_SECONDS` auto hang-up.
- Needs partition scheme "ESP SR 16M (3MB APP/7MB SPIFFS/2.9MB MODEL)" (PlatformIO: `esp_sr_16.csv`). Fails safe: if the listener can't start, the button still works.
- Mic I2S is now 32-bit stereo (left slot used) so the listener and Gemini streaming share one port.
- A stub-library compile check passes for wake word on/off; real library signatures (ESP_SR.begin/pause/resume, sr_cmd_t) are from memory and unverified. Custom wake words (Edge Impulse) are not implemented.
- `WAKE_ENGINE` (config.h): 1 = ESP-SR "Hi ESP"; 2 = empty slot in `wakeword.cpp` for a custom engine (Picovoice Porcupine or Edge Impulse). Robot Maker has a "Robot name" field (persona name; the wake phrase stays "Hi, ESP!"). Compile check passes for both engines, on/off.
- `WAKE_PHRASE` (config.h, default "Hi ESP"): "Hi ESP" -> WakeNet; any other phrase -> MultiNet text command in command mode (re-armed on timeout). The Robot Maker builds the phrase as "Hi " + robot name. Experimental: MultiNet accuracy for arbitrary names is unverified; ESP_SR.setMode/begin(mode) calls are from memory.
- Push button (AliExpress metal button) has C / NO / NC switch terminals plus LED terminals. Wiring: NO -> GPIO9, C -> GND; NC and the LED are unused. `arduino/Button_Test` helps identify the wires.
- Kids guide section 4 has a "What computer do you have?" picker (Mac Apple chip / Mac Intel / Windows). Per-computer text uses `data-for` attributes toggled by one `applyOS` script; the choice is saved in localStorage (`kids-os`); shortcut keys (`.mod`) show ⌘ or Ctrl. Windows is preselected on Windows browsers; on a Mac the chip is never guessed. Not checked on Windows or an Intel Mac.

## Update: big 2.4" colour screen replaces the OLED (current design)
- Why: the small SSD1306 OLED never answered on the author's board (the I2C scan found nothing on any pin), while a 2.4" 240x320 ILI9341 SPI colour screen worked first time on the same breadboard. The face is now widescreen (320x240, `TFT_ROTATION 1`; use 3 if upside down).
- Firmware: `src/face.cpp` draws the six moods in colour onto a hidden `GFXcanvas16` and blits it (no flicker); a coloured strip shows the mood word. Libraries: Adafruit GFX + Adafruit ILI9341. `src/tft_test.cpp` (colour bars, blink, 4 rotations) replaces the I2C scanner. There is no way to detect an SPI display that has no MISO wire, so the demo cannot say "screen found".
- Pins (all on the brain's usable row): mic SCK 4, WS 5, SD 6; amp BCLK 15, LRC 7, DIN 16; screen CS 13, RESET 12, DC 11, SDI/MOSI 10, SCK 9; button 14. Screen power: VCC to the 5V pin, LED (backlight) to the 3.3V rail, GND to the ground rail. The amp's VIN borrows 5V with a short wire from the screen's VCC column.
- Breadboard Buddy: 15 steps; nested lanes (screen farthest, then amp, then mic) so only one wire crossing remains. `node tools/verify_breadboard_design.js guide-breadboard.html include/config.h` checks the picture against config.h.
- Unverified on hardware: the new face.cpp, the pin set (the author tested the screen on CS 10, MOSI 11, SCK 12, DC 14, RESET 9, which is a different set), and the real Adafruit library calls (checked only against stand-in stubs).

## Update: new breadboard, Makeronics BB-102 (current design)
- The author's first breadboard had bad rows, so the project moved to the Makeronics BB-102 (Amazon B07Q34YND5; details from the photo `images/breadboard.jpg`, the listing page could not be opened). It has TWO strips of 61 columns (A-E and F-J around the groove), FOUR power-rail pairs, and every rail is CUT in two halves after column 30.
- Breadboard Buddy layout: the brain on strip 1 with the free row at the BOTTOM (USB on the right, row reads `3V3, 3V3, RST, 4, 5, 6, 7, 15, 16, 17, 18, 8, 3, 46, 9, 10, 11, 12, 13, 14, 5V, G`, free holes in row j); mic (cols 1-6), amp (8-14) and screen (16-29) on strip 2, pins in row e, wires in row b. Wire lanes run in the gap between the strips.
- Power: brain G -> strip 1 blue rail, brain 3V3 (col 2) -> strip 1 red rail, then one bridge wire each to the strip 2 rails (cols 12 and 11). All rail holes are kept in columns 1-30, so no rail needs a bridge across the cut. The button sits beside the brain: NO -> pin 14, C -> strip 1 blue rail.
- The pins and the firmware did not change. Screen VCC is still on the 5V pin; the author's backlight lit with VCC on 3V3, so VCC may move to the red rail once the full face is confirmed on 3.3V.
- `tools/verify_breadboard_design.js` now checks two strips, the rail cut, wire crossings with real geometry (currently 0) and the pin set. The new picture was only checked by script (and a syntax check); it has not been looked at in a browser by the assistant, and the wiring has not been tried on the real board.
