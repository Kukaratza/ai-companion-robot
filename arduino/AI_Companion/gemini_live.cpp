#include "gemini_live.h"
#include <WebSocketsClient.h>
#include <mbedtls/base64.h>
#include "audio_out.h"
#include "config.h"
#include "secrets.h"

static WebSocketsClient ws;
static bool active = false, ready = false;
static String msg;  // reassembly buffer for fragmented messages

bool geminiReady() { return ready; }
bool geminiActive() { return active; }

static void sendSetup() {
    String s = "{\"setup\":{\"model\":\"" GEMINI_MODEL "\","
               "\"generationConfig\":{\"responseModalities\":[\"AUDIO\"],"
               "\"speechConfig\":{\"voiceConfig\":{\"prebuiltVoiceConfig\":{\"voiceName\":\"" GEMINI_VOICE "\"}}}},"
               "\"systemInstruction\":{\"parts\":[{\"text\":\"" SYSTEM_PROMPT "\"}]}}}";
    ws.sendTXT(s);
}

// Decode every "data":"<base64>" in the message and queue it for playback
static void handleMessage(const String &m) {
    if (m.indexOf("setupComplete") >= 0) {
        ready = true;
        Serial.println("Gemini ready");
    }
    if (m.indexOf("\"interrupted\"") >= 0) audioOutClear();
    if (m.indexOf("turnComplete") >= 0) Serial.println("turn complete");

    int pos = 0;
    const String key = "\"data\":\"";
    while ((pos = m.indexOf(key, pos)) >= 0) {
        pos += key.length();
        int end = m.indexOf('"', pos);
        if (end < 0) break;
        size_t inLen = end - pos, outLen = 0;
        size_t cap = inLen * 3 / 4 + 4;
        uint8_t *pcm = (uint8_t *)ps_malloc(cap);
        if (pcm && mbedtls_base64_decode(pcm, cap, &outLen,
                                         (const uint8_t *)m.c_str() + pos, inLen) == 0) {
            audioOutPush((int16_t *)pcm, outLen / 2);
        }
        free(pcm);
        pos = end;
    }
}

static void onEvent(WStype_t type, uint8_t *payload, size_t length) {
    switch (type) {
        case WStype_CONNECTED:
            Serial.println("WS connected");
            sendSetup();
            break;
        case WStype_DISCONNECTED:
            Serial.println("WS disconnected");
            ready = false;
            active = false;
            break;
        case WStype_TEXT:
        case WStype_BIN: {
            String m;
            m.concat((const char *)payload, length);
            handleMessage(m);
            break;
        }
        case WStype_FRAGMENT_TEXT_START:
        case WStype_FRAGMENT_BIN_START:
            msg = "";
            msg.concat((const char *)payload, length);
            break;
        case WStype_FRAGMENT:
            msg.concat((const char *)payload, length);
            break;
        case WStype_FRAGMENT_FIN:
            msg.concat((const char *)payload, length);
            handleMessage(msg);
            msg = "";
            break;
        default:
            break;
    }
}

void geminiBegin() {
    if (active) return;
    active = true;
    ready = false;
    ws.beginSSL("generativelanguage.googleapis.com", 443,
                "/ws/google.ai.generativelanguage.v1beta.GenerativeService.BidiGenerateContent?key=" GEMINI_API_KEY);
    ws.onEvent(onEvent);
    ws.setReconnectInterval(0);
    ws.enableHeartbeat(15000, 3000, 2);
}

void geminiEnd() {
    ws.disconnect();
    ready = false;
    active = false;
    audioOutClear();
}

void geminiLoop() { ws.loop(); }

void geminiSendAudio(const int16_t *samples, size_t count) {
    if (!ready) return;
    size_t bytes = count * 2;
    size_t cap = (bytes + 2) / 3 * 4 + 1, outLen = 0;
    static char b64[4400];  // enough for 100 ms (3200 B -> 4268 B)
    if (cap > sizeof(b64)) return;
    mbedtls_base64_encode((uint8_t *)b64, sizeof(b64), &outLen, (const uint8_t *)samples, bytes);
    b64[outLen] = 0;
    String s = "{\"realtimeInput\":{\"audio\":{\"mimeType\":\"audio/pcm;rate=16000\",\"data\":\"";
    s += b64;
    s += "\"}}}";
    ws.sendTXT(s);
}
