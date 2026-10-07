#pragma once
#include <Arduino.h>

void audioOutBegin();
// Queue 16-bit mono PCM (24 kHz) for playback
void audioOutPush(const int16_t *samples, size_t count);
// Drop everything queued (used on interruption / session end)
void audioOutClear();
// Samples still waiting to be played
size_t audioOutPending();
