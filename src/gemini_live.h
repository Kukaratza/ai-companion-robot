#pragma once
#include <Arduino.h>

// Opens a Gemini Live session (non-blocking; connection completes in loop()).
void geminiBegin();
void geminiEnd();
bool geminiReady();      // setup finished, OK to stream audio
bool geminiActive();     // session open or opening
void geminiLoop();       // call often
void geminiSendAudio(const int16_t *samples, size_t count);  // 16 kHz PCM
