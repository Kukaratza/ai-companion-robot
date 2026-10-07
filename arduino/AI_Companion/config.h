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
// INMP441 microphone (the EAR) - the brain's BOTTOM row of pins
#define PIN_MIC_SCK 42
#define PIN_MIC_WS  41
#define PIN_MIC_SD  40

// MAX98357 amplifier (the VOICE BOX) - the brain's TOP row of pins
#define PIN_SPK_BCLK 15
#define PIN_SPK_LRC  16
#define PIN_SPK_DIN  7

// The big push button. Push it to start or stop a conversation.
#define PIN_BUTTON 4

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

// --- The face (the little screen) ---
// 1 = use the face, 0 = run without it
#define FACE_ENABLED 1
#define PIN_OLED_SDA 2
#define PIN_OLED_SCL 1
#define OLED_ADDR    0x3C
// The screen sits upside down on the breadboard, so turn its picture around.
// (1 = turn around, 0 = leave it)
#define FACE_FLIP    1
