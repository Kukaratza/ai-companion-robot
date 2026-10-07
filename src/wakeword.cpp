// THE SLEEPING EAR: listens for the robot's name all by itself, inside the brain.
// Nothing goes to the internet until the name is heard.
//
// There are 2 "engines" (listeners). Pick one with WAKE_ENGINE in config.h:
//   1 = the built-in listener. It hears WAKE_PHRASE from config.h, like "Hi ESP"
//       or "Hi Jarvis".
//   2 = your own wake word. This is a project for a teen and a grown-up. The
//       4 functions are empty, ready for you to fill in (see "ENGINE 2" below).
#include "wakeword.h"
#include "config.h"

#if WAKEWORD_ENABLED && WAKE_ENGINE == 1
// ============== ENGINE 1: the phrase in WAKE_PHRASE ==============
// "Hi ESP" uses the chip's best listener (WakeNet). Any other phrase, like
// "Hi Jarvis", uses the chip's phrase-understanding listener (MultiNet). That one
// reads your words as plain English, so it can make mistakes with unusual names.
#include <ESP_SR.h>  // Espressif's speech-recognition toolbox (comes with the ESP32 board package)
#include <strings.h>
#include "audio_in.h"

static volatile bool heard = false;  // did we hear the phrase?
static bool running = false;         // is the listener switched on?
static bool phraseMode = false;      // true = listening for our own phrase (not "Hi ESP")

// The list of phrases to listen for. We only need one: the wake-up phrase.
static const sr_cmd_t commands[] = {{0, WAKE_PHRASE}};

// The listener calls this function whenever something happens.
static void onSpeechEvent(sr_event_t event, int command_id, int phrase_id) {
    if (event == SR_EVENT_WAKEWORD) heard = true;                    // it heard "Hi ESP"!
    else if (event == SR_EVENT_COMMAND) heard = true;                // it heard our own phrase!
    else if (event == SR_EVENT_TIMEOUT && phraseMode) ESP_SR.setMode(SR_MODE_COMMAND);  // keep listening
}

bool wakeBegin() {
    phraseMode = strcasecmp(WAKE_PHRASE, "Hi ESP") != 0;
    ESP_SR.onEvent(onSpeechEvent);
    running = ESP_SR.begin(audioInI2S(), commands, sizeof(commands) / sizeof(sr_cmd_t),
                           SR_CHANNELS_STEREO, phraseMode ? SR_MODE_COMMAND : SR_MODE_WAKEWORD);
    return running;
}

bool wakeHeard() {
    if (running && heard) {
        heard = false;  // only say "yes" one time
        return true;
    }
    return false;
}

void wakePause() {
    if (running) ESP_SR.pause();
}

void wakeResume() {
    if (running) {
        heard = false;
        ESP_SR.resume();
        if (phraseMode) ESP_SR.setMode(SR_MODE_COMMAND);
    }
}

#elif WAKEWORD_ENABLED && WAKE_ENGINE == 2
// ================= ENGINE 2: YOUR OWN WAKE WORD =================
// Fill in the 4 functions below. The rest of the robot already knows how to use them:
//   wakeBegin()  - runs once at start-up. Load your model. Return true if it worked.
//   wakeHeard()  - return true ONE time when your wake word was heard.
//   wakePause()  - stop listening (a conversation with Gemini just started).
//   wakeResume() - start listening again (the conversation ended).
//
// Ideas for how to make your own wake word (ask a grown-up to help):
//   * Picovoice Porcupine: type any word into their website and download a model.
//   * Edge Impulse: record your word many times, train a model in your browser,
//     then export it as an Arduino library (Sketch > Include Library > Add .ZIP Library).
//
// To hear the microphone, use audioInRead() from audio_in.h. It gives you 16,000
// numbers per second. Only read it while listening. While a conversation is going,
// the main file reads the microphone, so wakePause() must stop your reading.
#include "audio_in.h"

static volatile bool heard = false;   // set this to true when your model hears the word
static volatile bool listening = false;

bool wakeBegin() {
    Serial.println("Custom wake word is not set up yet - see wakeword.cpp (use the button for now)");
    return false;  // change to true when your listener is ready
}

bool wakeHeard() {
    if (listening && heard) {
        heard = false;
        return true;
    }
    return false;
}

void wakePause() { listening = false; }

void wakeResume() {
    heard = false;
    listening = true;
}

#else
// ============ wake word is switched off: these do nothing ============
bool wakeBegin() { return false; }
bool wakeHeard() { return false; }
void wakePause() {}
void wakeResume() {}
#endif
