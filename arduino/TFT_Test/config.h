#pragma once
// ==========================================================
//  MAKE IT YOURS  (the fun part! Change these settings)
// ==========================================================
// 1) WHO is your robot? Change the words between the quotes.
#define SYSTEM_PROMPT "You are a friendly desktop companion. Keep answers short, fun and kid-friendly."
// 2) What VOICE does it have? Try "Puck", "Kore", "Charon", "Fenrir" or "Aoede".
#define GEMINI_VOICE "Puck"
// 3) How LOUD is it? 0 = silent, 100 = very loud.
#define SPK_VOLUME 60
// 4) Wake up when you say a phrase? 1 = yes, 0 = no (use the button).
#define WAKEWORD_ENABLED 0
// 5) The wake-up phrase: "Hi" + your robot's name, like "Hi Jarvis". Use plain English
//    words that sound the way they are spelled. "Hi ESP" works best.
#define WAKE_PHRASE "Hi ESP"
// ==========================================================

// ==========================================================
//  SETTINGS - a list of labels (names) the robot uses.
//  "#define NAME value" means: wherever the robot sees NAME,
//  use value. Change a value here to change the robot!
// ==========================================================

// --- Which pin is each part plugged into? ---
// All of them are on the brain's pin row that sits next to the breadboard's free holes.
// INMP441 microphone (the EAR)
#define PIN_MIC_SCK 4
#define PIN_MIC_WS  5
#define PIN_MIC_SD  6

// MAX98357 amplifier (the VOICE BOX)
#define PIN_SPK_BCLK 15
#define PIN_SPK_LRC  7
#define PIN_SPK_DIN  16

// The big push button. Push it to start or stop a conversation.
#define PIN_BUTTON 14

// --- Sound settings (please leave these alone) ---
#define MIC_RATE_HZ     16000   // how many sound snapshots per second going IN to Gemini
#define SPK_RATE_HZ     24000   // how many sound snapshots per second coming OUT of Gemini
#define MIC_CHUNK_SAMPLES 1600  // how many snapshots we send at once (0.1 seconds)

// --- Gemini settings ---
// Verify against Google's current Live API docs
#define GEMINI_MODEL "models/gemini-2.5-flash-native-audio-preview-09-2025"

// --- Microphone sensitivity ---
// Mic too quiet? Make MIC_SHIFT smaller (like 12). Too loud? Make it bigger (like 16).
#define MIC_SHIFT 14

// --- Say its name (only when WAKEWORD_ENABLED is 1) ---
// Which listener? 1 = the built-in one. It hears WAKE_PHRASE ("Hi ESP" works best).
//                 2 = YOUR OWN trained wake word (a teen project, see wakeword.cpp)
#define WAKE_ENGINE 1
// After this many quiet seconds the robot hangs up and goes back to sleep.
#define SESSION_IDLE_SECONDS 15

// --- The face (the big 2.4" colour screen, 240 x 320, ILI9341) ---
// 1 = use the face, 0 = run without it
#define FACE_ENABLED 1
// The screen's 5 data wires. The names on the screen's pins are in the comments.
#define PIN_TFT_CS   13   // screen pin CS
#define PIN_TFT_RST  12   // screen pin RESET
#define PIN_TFT_DC   11   // screen pin DC
#define PIN_TFT_MOSI 10   // screen pin SDI (MOSI)
#define PIN_TFT_SCK  9    // screen pin SCK
// How fast the data wires talk. If you see static or strange colours, try 4000000.
#define TFT_SPI_HZ   10000000
// The face is WIDE (320 across), so use 1 or 3 only. If the face is upside down, change it
// to the other one. (0 and 2 are the tall screen and are not allowed.)
#define TFT_ROTATION 1
