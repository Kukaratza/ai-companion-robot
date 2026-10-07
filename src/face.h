#pragma once
#include <Arduino.h>

enum FaceMood {
    FACE_IDLE,    // open eyes, random blinks, looks around
    FACE_HAPPY,   // smiling ^ ^ eyes
    FACE_LISTEN,  // wide eyes + sound dots
    FACE_THINK,   // eyes look side to side + "..."
    FACE_SPEAK,   // eyes + moving mouth
    FACE_SLEEP    // closed eyes + z
};

// Starts the face animation task. Returns false (and does nothing) if no
// OLED is found, so the rest of the robot keeps working without a screen.
bool faceBegin();
void faceSet(FaceMood m);  // cheap; safe to call every loop
