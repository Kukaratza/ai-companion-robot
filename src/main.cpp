// ==========================================================
//  MY TALKING ROBOT - the main file (the "boss" of the robot)
//  Press the button, talk, and the robot answers out loud!
// ==========================================================

// These lines bring in "toolboxes" that other people (and we) wrote.
#include <Arduino.h>      // the basic Arduino tools
#include <WiFi.h>         // lets the robot use the internet
#include "audio_in.h"     // our EAR (the microphone)
#include "audio_out.h"    // our MOUTH (the speaker)
#include "config.h"       // our settings
#include "face.h"         // our FACE (the little screen)
#include "gemini_live.h"  // the phone line to Gemini
#include "secrets.h"      // our WiFi name and password
#include "wakeword.h"     // the sleeping ear that listens for "Hi, ESP!"

// A "variable" is a labelled box that remembers something.
// This box remembers if the button was up (HIGH) or pushed down (LOW).
static bool lastButton = HIGH;
static bool wasActive = false;     // were we in a conversation a moment ago?
static uint32_t lastActive = 0;    // the last time someone was talking

// A "function" is a mini recipe with a name.
// This one answers the question: "was the button JUST pressed?"
static bool buttonPressed() {
    bool b = digitalRead(PIN_BUTTON);                 // look at the button right now
    bool pressed = (lastButton == HIGH && b == LOW);  // it was up, and now it is down
    lastButton = b;                                   // remember it for next time
    if (pressed) delay(30);                           // wait a tiny bit so one push counts once
    return pressed;
}

// setup() runs ONE time, when the robot wakes up.
void setup() {
    Serial.begin(115200);  // open the robot's "diary" so it can write messages
    delay(500);
    Serial.printf("PSRAM: %u bytes\n", ESP.getPsramSize());  // write how much memory we have
    pinMode(PIN_BUTTON, INPUT_PULLUP);                       // get the button ready

#if FACE_ENABLED
    faceBegin();  // wake up the face. It says hello with a happy smile!
#endif

    WiFi.begin(WIFI_SSID, WIFI_PASS);  // join the WiFi
    while (WiFi.status() != WL_CONNECTED) {  // keep waiting until we are connected
        delay(300);
        Serial.print('.');  // write a dot in the diary every time we wait
    }
    Serial.printf("\nWiFi OK: %s\n", WiFi.localIP().toString().c_str());

    audioInBegin();   // get the ear ready
    audioOutBegin();  // get the mouth ready
#if WAKEWORD_ENABLED
    if (wakeBegin()) Serial.println("Say \"Hi, ESP!\" to wake me up (or press the button)");
    else Serial.println("Wake word is not working - use the button (check the Partition Scheme)");
#else
    Serial.println("Press the button to talk");
#endif
}

// loop() runs OVER and OVER, forever, super fast.
void loop() {
    // 1) Was the button pressed? Then start or stop a conversation.
    if (buttonPressed()) {
        if (geminiActive()) {
            geminiEnd();  // we were talking, so hang up
            Serial.println("Session ended");
        } else {
            geminiBegin();  // we were not talking, so call Gemini
            Serial.println("Session starting");
        }
    }

    geminiLoop();  // keep the phone line working

    // Was the robot's name heard while it was sleeping? Then call Gemini!
    if (!geminiActive() && wakeHeard()) {
        Serial.println("I heard my name!");
        geminiBegin();
        lastActive = millis();
    }

    // When a call starts, stop listening for the name. When it ends, listen again.
    bool active = geminiActive();
    if (active && !wasActive) { wakePause(); lastActive = millis(); }
    if (!active && wasActive) wakeResume();
    wasActive = active;

#if FACE_ENABLED
    // 2) Choose the right face for what the robot is doing right now.
    if (!geminiActive())        faceSet(WAKEWORD_ENABLED ? FACE_SLEEP : FACE_IDLE);  // waiting for you
    else if (!geminiReady())    faceSet(FACE_THINK);   // calling Gemini...
    else if (audioOutPending()) faceSet(FACE_SPEAK);   // the robot is talking
    else                        faceSet(FACE_LISTEN);  // the robot is listening
#endif

    // 3) If the call is ready, send what the microphone hears to Gemini.
    if (geminiReady()) {
        static int16_t chunk[MIC_CHUNK_SAMPLES];           // a little bucket of sound
        size_t n = audioInRead(chunk, MIC_CHUNK_SAMPLES);  // fill it from the microphone
        // Don't send sound while the robot is talking, or it would hear itself!
        if (n && audioOutPending() == 0) geminiSendAudio(chunk, n);

#if WAKEWORD_ENABLED
        // Keep track of whether anyone is talking. If it is quiet for a while,
        // hang up and go back to sleep.
        long loud = 0;
        for (size_t i = 0; i < n; i++) loud += abs(chunk[i]);
        if (n && (loud / (long)n > 400 || audioOutPending())) lastActive = millis();
        if (millis() - lastActive > SESSION_IDLE_SECONDS * 1000UL) {
            geminiEnd();
            Serial.println("Quiet for a while - going back to sleep");
        }
#endif
    } else {
        delay(5);
    }
}
