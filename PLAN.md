# Desktop AI Companion (ESP32-S3 + Gemini) — Plan

## Context
Build a standalone desktop voice assistant. The ESP32-S3 listens for a wake word, streams mic audio directly to the Gemini Live API over WiFi, and plays the spoken reply. Choices: ESP32 direct (no laptop relay), wake word trigger, no servo for now, PlatformIO/Arduino firmware. The project directory is currently empty.

## Parts used / not used
- Used: ESP32-S3-N16R8 (8MB PSRAM is required for TLS + audio buffers), INMP441 mic, MAX98357 amp + 3W 4Ω speaker, breadboard + MB102 power, one push button (mute/reset fallback), jumper wires.
- Optional later: TP4056 + 18650 (battery), MG90S servo, TTP223, N20 motor.
- Not needed: N20 motor. Servo is deferred.
- Battery caveat: the TP4056 has no boost regulator. 3.7V into the dev board's 5V pin will brown out the LDO. Develop on USB power. For battery use, add a 5V boost converter (not in your list).

## Wiring (ESP32-S3, separate I2S buses)
- INMP441 (I2S0, RX): SCK→GPIO42, WS→GPIO41, SD→GPIO40, L/R→GND, VDD→3V3, GND→GND. (Original plan used GPIO4/5/6; moved to the bottom pin row, see the update at the end.)
- MAX98357 (I2S1, TX): BCLK→GPIO15, LRC→GPIO16, DIN→GPIO7, VIN→5V, GND→GND, SD tied to VIN (always on, or a GPIO for mute). Speaker to its +/- outputs.
- Button: GPIO4 to GND (INPUT_PULLUP). (Original plan: GPIO9.)
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

## Update: robot face (added later)
- Part: 0.96" SSD1306 OLED, 128x64, 4-pin I2C (pins GND VDD SCK SDA), yellow/blue or white/blue two-colour: top 16 rows are one colour, the rest blue.
- Wiring: GND→GND, VDD→3V3, SCK (=SCL)→GPIO1, SDA→GPIO2 (I2C address 0x3C). Pins/address in `include/config.h`.
- Firmware: `src/face.{h,cpp}` runs a 20 fps animation task (moods: idle, happy, listen, think, speak, sleep); `main.cpp` sets the mood from session state. Optional: `FACE_ENABLED 0`, and it skips itself if no OLED is found.
- Kid-first milestone: `pio run -e face_demo -t upload` builds `src/face_demo.cpp`, which needs only ESP32 + OLED (no mic, speaker or WiFi). Cycles through every mood.
- Libraries: Adafruit SSD1306 + Adafruit GFX. Not yet compiled or run on hardware.
- The face uses the top 16-pixel band for a status word (READY, LISTENING, ...) and draws the eyes below it.

## Update: pins split across both sides of the ESP32 (less wire clutter)
- Top row of pins (left header): amplifier BCLK=15, LRC=16, DIN=7; button=GPIO4; 5V and GND.
- Bottom row of pins (right header): mic SCK=42, WS=41, SD=40; face SDA=2, SCL=1; GND; 3V3 is fed around the left end to the bottom red rail.
- Breadboard: amp on the top half (upright); mic and face on the bottom half, flipped over. `FACE_FLIP 1` rotates the OLED picture 180 degrees.
- Pin names from the usual ESP32-S3-DevKitC-1 layout; verify against the labels on your board.

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
