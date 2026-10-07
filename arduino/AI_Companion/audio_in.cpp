// THE EAR: reads sound from the microphone and turns it into numbers.
#include "audio_in.h"
#include "config.h"

static I2SClass mic;  // "mic" is our name for the microphone connection

// Other files (like the wake-word listener) share the same microphone.
I2SClass &audioInI2S() { return mic; }

// Get the microphone ready. Runs once from setup().
void audioInBegin() {
    // Tell the brain which 3 pins the microphone is plugged into.
    mic.setPins(PIN_MIC_SCK, PIN_MIC_WS, -1, PIN_MIC_SD);
    // Start listening. The microphone sends a LEFT and a RIGHT sound. Ours only
    // talks on the left. If starting does not work, write a message in the diary.
    if (!mic.begin(I2S_MODE_STD, MIC_RATE_HZ, I2S_DATA_BIT_WIDTH_32BIT,
                   I2S_SLOT_MODE_STEREO, I2S_STD_SLOT_BOTH)) {
        Serial.println("Mic I2S init failed");
    }
}

// Listen for a moment and put the sound into a bucket called "out".
size_t audioInRead(int16_t *out, size_t count) {
    static int32_t raw[MIC_CHUNK_SAMPLES * 2];  // a bucket for the raw, big numbers (left + right)
    if (count > MIC_CHUNK_SAMPLES) count = MIC_CHUNK_SAMPLES;
    size_t frames = mic.readBytes((char *)raw, count * 2 * sizeof(int32_t)) / (2 * sizeof(int32_t));
    // The microphone gives BIG numbers. Gemini wants smaller ones,
    // so we shrink every number a little (MIC_SHIFT says how much).
    // raw[i * 2] is the LEFT number. We skip the empty right one.
    for (size_t i = 0; i < frames; i++) {
        int32_t s = raw[i * 2] >> MIC_SHIFT;
        out[i] = (int16_t)constrain(s, -32768, 32767);  // keep it inside the allowed range
    }
    return frames;
}
