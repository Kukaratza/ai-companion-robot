#pragma once
#include <Arduino.h>

// "Say its name": the robot sleeps until it hears "Hi, ESP!"
// Everything here does nothing when WAKEWORD_ENABLED is 0 in config.h.
bool wakeBegin();   // start listening for the name. Returns false if it cannot.
bool wakeHeard();   // true ONE time after the robot hears its name
void wakePause();   // stop listening (while the robot is in a conversation)
void wakeResume();  // listen for the name again
