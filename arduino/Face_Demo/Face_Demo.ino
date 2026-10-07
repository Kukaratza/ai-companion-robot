// STEP 1 FOR KIDS: only the ESP32 + the OLED screen. No mic, speaker or WiFi needed.
// Upload with:  pio run -e face_demo -t upload
#include <Arduino.h>
#include "face.h"

static const FaceMood order[] = {FACE_HAPPY, FACE_IDLE, FACE_LISTEN, FACE_THINK, FACE_SPEAK, FACE_SLEEP};
static const char *names[] = {"happy", "idle", "listening", "thinking", "speaking", "sleeping"};

void setup() {
    Serial.begin(115200);
    if (!faceBegin()) Serial.println("Check the 4 OLED wires: GND, VCC, SCL, SDA");
}

void loop() {
    // Show every mood for a few seconds, then repeat
    for (int i = 0; i < 6; i++) {
        Serial.printf("Face: %s\n", names[i]);
        faceSet(order[i]);
        delay(i == 0 ? 2500 : 4000);
    }
}
