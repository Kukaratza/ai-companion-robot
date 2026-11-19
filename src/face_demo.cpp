// STEP 1 FOR KIDS: only the ESP32 + the big colour screen. No mic, speaker or WiFi needed.
// Upload with:  pio run -e face_demo -t upload
#include <Arduino.h>
#include "face.h"

static const FaceMood order[] = {FACE_HAPPY, FACE_IDLE, FACE_LISTEN, FACE_THINK, FACE_SPEAK, FACE_SLEEP};
static const char *names[] = {"happy", "idle", "listening", "thinking", "speaking", "sleeping"};

void setup() {
    Serial.begin(115200);
    delay(2500);  // wait for the Serial Monitor to connect, so no message gets lost
    if (faceBegin()) Serial.println("Face started. The screen should light up now!");
    else Serial.println("Face could not start. See the message above.");
}

void loop() {
    // Show every mood for a few seconds, then repeat
    for (int i = 0; i < 6; i++) {
        Serial.printf("Face: %s\n", names[i]);
        faceSet(order[i]);
        delay(i == 0 ? 2500 : 4000);
    }
}
