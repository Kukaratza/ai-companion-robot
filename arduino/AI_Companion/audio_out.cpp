#include "audio_out.h"
#include <ESP_I2S.h>
#include "config.h"

static I2SClass spk;
static const size_t RING_SAMPLES = SPK_RATE_HZ * 20;  // 20 s in PSRAM
static int16_t *ring;
static volatile size_t head = 0, tail = 0;
static portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;

static size_t pendingLocked() { return (head + RING_SAMPLES - tail) % RING_SAMPLES; }

size_t audioOutPending() {
    portENTER_CRITICAL(&mux);
    size_t n = pendingLocked();
    portEXIT_CRITICAL(&mux);
    return n;
}

void audioOutClear() {
    portENTER_CRITICAL(&mux);
    tail = head;
    portEXIT_CRITICAL(&mux);
}

void audioOutPush(const int16_t *samples, size_t count) {
    for (size_t i = 0; i < count; i++) {
        portENTER_CRITICAL(&mux);
        size_t next = (head + 1) % RING_SAMPLES;
        if (next != tail) {  // drop on overflow
            ring[head] = (int16_t)((samples[i] * SPK_VOLUME) / 100);
            head = next;
        }
        portEXIT_CRITICAL(&mux);
    }
}

static void playbackTask(void *) {
    static int16_t buf[480];  // 20 ms
    for (;;) {
        size_t avail = audioOutPending();
        if (avail == 0) {
            vTaskDelay(pdMS_TO_TICKS(5));
            continue;
        }
        size_t n = min(avail, (size_t)480);
        portENTER_CRITICAL(&mux);
        for (size_t i = 0; i < n; i++) {
            buf[i] = ring[tail];
            tail = (tail + 1) % RING_SAMPLES;
        }
        portEXIT_CRITICAL(&mux);
        spk.write((uint8_t *)buf, n * sizeof(int16_t));  // blocks at DMA rate
    }
}

void audioOutBegin() {
    ring = (int16_t *)ps_malloc(RING_SAMPLES * sizeof(int16_t));
    spk.setPins(PIN_SPK_BCLK, PIN_SPK_LRC, PIN_SPK_DIN);
    if (!spk.begin(I2S_MODE_STD, SPK_RATE_HZ, I2S_DATA_BIT_WIDTH_16BIT,
                   I2S_SLOT_MODE_MONO, I2S_STD_SLOT_LEFT)) {
        Serial.println("Speaker I2S init failed");
        return;
    }
    xTaskCreatePinnedToCore(playbackTask, "playback", 4096, nullptr, 3, nullptr, 1);
}
