#pragma once
#include <Arduino.h>
#include <ESP_I2S.h>

void audioInBegin();
// Blocking read of `count` 16-bit samples at 16 kHz. Returns samples read.
size_t audioInRead(int16_t *out, size_t count);
// The microphone connection itself (the wake-word listener shares it)
I2SClass &audioInI2S();
