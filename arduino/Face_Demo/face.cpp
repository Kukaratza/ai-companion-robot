// THE FACE: draws the robot's eyes and mouth on the big colour screen.
// It fits itself to the screen, so it works for every TFT_ROTATION: 0 and 2 (tall) or 1 and 3 (wide).
// FACE_SMOOTH 1: each picture is drawn on a hidden board and shown in ONE go (no flicker).
// FACE_SMOOTH 0: draws straight onto the screen (the way TFT_Test does) and may flicker.
#include "face.h"
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include "config.h"

// If an older config.h is missing the newer face settings, use these.
#ifndef FACE_SMOOTH
#define FACE_SMOOTH 1
#endif
#ifndef FACE_SIZE_PERCENT
#define FACE_SIZE_PERCENT 105
#endif
#ifndef FACE_SHIFT_X
#define FACE_SHIFT_X 0
#endif
#ifndef FACE_SHIFT_Y
#define FACE_SHIFT_Y 0
#endif

// Make a colour from amounts of red, green and blue (0 to 255 each).
#define RGB565(r, g, b) ((((r) & 0xF8) << 8) | (((g) & 0xFC) << 3) | ((b) >> 3))
#define BLACK 0x0000

// One colour for every mood. The order matches the list in face.h.
static const uint16_t COLOUR[] = {
    RGB565(0, 220, 255),    // waiting: cyan eyes
    RGB565(255, 120, 190),  // hello: pink eyes
    RGB565(90, 255, 140),   // listening: green eyes
    RGB565(190, 140, 255),  // thinking: purple eyes
    RGB565(255, 220, 60),   // talking: yellow eyes
    RGB565(110, 150, 255),  // sleeping: blue eyes
};

static Adafruit_ILI9341 tft(&SPI, PIN_TFT_DC, PIN_TFT_CS, PIN_TFT_RST);  // "tft" is our name for the screen
static volatile FaceMood mood = FACE_IDLE;  // which face are we making right now?
static volatile uint32_t moodSince = 0;     // when did this face start?
static GFXcanvas16 *board = nullptr;        // the hidden drawing board (only when FACE_SMOOTH is 1)
static int bx, by, bw, bh;                  // where the hidden board sits on the screen
static int gx = 0, gy = 0;                  // shift from screen dots to dots on whatever we draw on

// Where the face goes. The face is drawn on a 280 x 184 "design" picture, then scaled up or
// down so it fills the whole screen. (0, 0) of the design is the top-left corner of the face.
static int W, H;           // screen width and height (they swap when the picture turns)
static float sc;           // how much to scale the design
static int ox, oy;         // where the design starts on the screen

// Other files call this to change the face, like faceSet(FACE_HAPPY).
void faceSet(FaceMood m) {
    if (m != mood) {
        mood = m;
        moodSince = millis();  // millis() = how many milliseconds since the robot woke up
    }
}

// Turn a design number into a screen number.
static inline int S(float v) { return (int)(v * sc + 0.5f); }
static inline int PX(float x) { return ox + S(x) + gx; }
static inline int PY(float y) { return oy + S(y) + gy; }

// Everything that can change from one frame to the next. When it is the same as last time,
// we don't need to draw anything.
struct Pose {
    int lookX, lookY;  // where the eyes look
    int blink;         // how open the eyes are, in percent (100 = wide open)
    int sway;          // thinking: eyes swing side to side
    int dots;          // listening / thinking: dots on the screen
    int mouth;         // speaking: how open the mouth is
    int zx;            // sleeping: where the z floats
};

// Draw ONE eye: a rounded rectangle. cx, cy = its centre (design numbers).
static void eye(Adafruit_GFX &g, float cx, float cy, float w, float h, uint16_t colour) {
    int ew = S(w), eh = S(h);
    if (eh < 6) eh = 6;  // never thinner than a line
    g.fillRoundRect(PX(cx) - ew / 2, PY(cy) - eh / 2, ew, eh, min(S(24), eh / 2), colour);
}

// Draw the moving parts of one mood. With colour == BLACK it rubs them out again.
static void paint(Adafruit_GFX &g, FaceMood m, const Pose &p, bool erase) {
    const int LX = 70, RX = 210, CY = 78;  // design: LEFT eye, RIGHT eye and eye height
    const float ew = 76, eh = 84;          // design: eye width and height
    const uint16_t c = erase ? BLACK : COLOUR[m];
    const float bs = p.blink / 100.0f;
    switch (m) {
        case FACE_IDLE:  // waiting: open eyes that blink and look around
            eye(g, LX + p.lookX, CY + p.lookY, ew, eh * bs, c);
            eye(g, RX + p.lookX, CY + p.lookY, ew, eh * bs, c);
            break;
        case FACE_HAPPY:  // happy: ^ ^ eyes, a smile and rosy cheeks (they never move)
            for (int cx : {LX, RX}) {
                g.fillRoundRect(PX(cx - 36), PY(CY - 28), S(72), S(60), S(26), c);
                g.fillCircle(PX(cx), PY(CY + 32), S(40), BLACK);  // cut away the bottom to make a ^
                g.fillCircle(PX(cx + (cx < 140 ? -22 : 22)), PY(128), S(12), erase ? BLACK : RGB565(255, 90, 120));
            }
            g.fillCircle(PX(140), PY(128), S(30), c);      // a smile is a half moon:
            g.fillCircle(PX(140), PY(112), S(30), BLACK);  // a circle with the top bitten off
            break;
        case FACE_LISTEN:  // listening: big round eyes and pulsing dots
            eye(g, LX, CY - 6, ew + 8, (eh + 20) * bs, c);
            eye(g, RX, CY - 6, ew + 8, (eh + 20) * bs, c);
            for (int i = 0; i < 3; i++) g.fillCircle(PX(100 + i * 40), PY(160), S(i == p.dots ? 10 : 6), c);
            break;
        case FACE_THINK:  // thinking: eyes swing side to side and "..." appear
            eye(g, LX + p.sway, CY - 8, ew, eh * bs, c);
            eye(g, RX + p.sway, CY - 8, ew, eh * bs, c);
            for (int i = 0; i < p.dots; i++) g.fillCircle(PX(100 + i * 40), PY(160), S(9), c);
            break;
        case FACE_SPEAK:  // talking: a mouth that opens and closes
            eye(g, LX, CY - 10, ew, eh * bs, c);
            eye(g, RX, CY - 10, ew, eh * bs, c);
            g.fillRoundRect(PX(100), PY(140), S(80), S(p.mouth), S(10), erase ? BLACK : RGB565(255, 120, 120));
            break;
        case FACE_SLEEP:  // sleeping: thin closed eyes and a floating z
            g.fillRoundRect(PX(LX - 36), PY(CY + 4), S(72), S(8), S(4), c);
            g.fillRoundRect(PX(RX - 36), PY(CY + 4), S(72), S(8), S(4), c);
            g.setTextColor(c);
            g.setTextSize(max(2, S(5)));
            g.setCursor(PX(p.zx), PY(20));
            g.print("z");
            break;
    }
}

// This "task" runs on its own, all the time, like a little artist inside the robot.
static void faceTask(void *) {
    const float blinkScale[] = {0.7f, 0.35f, 0.1f, 0.35f, 0.7f};  // how closed the eye is, frame by frame
    int blinkFrame = -1;  // -1 means "not blinking right now"
    uint32_t nextBlink = millis() + 2500, nextLook = millis() + 3000;
    int lookX = 0, lookY = 0;  // how far the eyes look left/right and up/down
    int shownMood = -1;        // which mood is on the screen right now
    Pose last = {};            // what we drew last time

    for (;;) {  // forever...
        uint32_t now = millis();
        FaceMood m = mood;

        // HELLO only lasts 2.5 seconds, then we go back to READY.
        if (m == FACE_HAPPY && now - moodSince > 2500) {
            faceSet(FACE_IDLE);
            m = FACE_IDLE;
        }

        // Time to blink? Step through the blink frames one by one.
        if (blinkFrame < 0 && now > nextBlink) blinkFrame = 0;
        float bs = 1.0f;  // 1.0 = eye fully open
        if (blinkFrame >= 0) {
            bs = blinkScale[blinkFrame++];
            if (blinkFrame >= 5) {
                blinkFrame = -1;
                nextBlink = now + random(2000, 5500);  // blink again in 2 to 5 seconds
            }
        }
        // Every few seconds, look somewhere new.
        if (now > nextLook) {
            lookX = random(-16, 17);
            lookY = random(-6, 7);
            nextLook = now + random(1500, 4000);
        }

        Pose p = {};
        p.blink = (int)(bs * 100);
        switch (m) {
            case FACE_IDLE:   p.lookX = lookX; p.lookY = lookY; break;
            case FACE_LISTEN: p.dots = (now / 200) % 3; break;
            case FACE_THINK:  p.sway = (int)(22 * sin(now / 350.0)); p.dots = (now / 350) % 4; break;
            case FACE_SPEAK:  p.mouth = 12 + (int)(14 + 14 * sin(now / 90.0 + random(0, 3))); break;  // wobbles
            case FACE_SLEEP:  p.zx = 215 + (int)(8 * sin(now / 400.0)); break;
            default: break;
        }

        bool newMood = (shownMood != (int)m);
        if (newMood) {  // new mood? Wipe the whole screen black first.
            tft.fillRect(0, 0, W, H, BLACK);
            shownMood = (int)m;
        }
        if (newMood || memcmp(&p, &last, sizeof(Pose)) != 0) {  // only draw when something changed
            if (board) {  // smooth: draw on the hidden board, then show it all in one go
                board->fillScreen(BLACK);
                paint(*board, m, p, false);
                tft.startWrite();
                tft.setAddrWindow(bx, by, bw, bh);
                tft.writePixels(board->getBuffer(), bw * bh);
                tft.endWrite();
            } else if (newMood) {
                paint(tft, m, p, false);
            } else {
                paint(tft, m, last, true);  // rub out the old picture of the moving parts...
                paint(tft, m, p, false);    // ...and draw the new one
            }
            last = p;
        }
        vTaskDelay(pdMS_TO_TICKS(30));
    }
}

// Get the screen ready. The screen only LISTENS, so we cannot ask it "are you there?".
// If nothing shows up, check the wires.
bool faceBegin() {
    SPI.begin(PIN_TFT_SCK, -1, PIN_TFT_MOSI, -1);  // no MISO wire: the screen only listens
    tft.begin(TFT_SPI_HZ);
    // The screen library restarts the SPI wires with its OWN default pins, which would undo
    // our choice (and fight with pins 11 and 12). So we set our pins one more time.
    SPI.end();
    SPI.begin(PIN_TFT_SCK, -1, PIN_TFT_MOSI, -1);
    // Wipe the screen black in every direction first, so nothing from an older sketch is left over.
    for (uint8_t r = 0; r < 4; r++) {
        tft.setRotation(r);
        tft.fillScreen(BLACK);
    }
    tft.setRotation(TFT_ROTATION & 3);  // 0 to 3: turn the picture until it looks right
    W = tft.width();
    H = tft.height();
    // Make the face as big as the screen allows: it is drawn on a 280 x 184 design picture.
    // FACE_SIZE_PERCENT makes it bigger or smaller. The eyes and dots sit around the point
    // (140, 101) of the design, so that point goes to the middle of the screen.
    sc = min(W / 280.0f, H / 184.0f) * (FACE_SIZE_PERCENT / 100.0f);
    ox = W / 2 - S(140) + FACE_SHIFT_X;
    oy = H / 2 - S(101) + FACE_SHIFT_Y;
#if FACE_SMOOTH
    // The hidden board only needs to cover where the face is drawn (design x 20..260, y 10..176).
    bx = max(0, ox + S(20));
    by = max(0, oy + S(10));
    bw = min(W - bx, ox + S(260) - bx);
    bh = min(H - by, oy + S(176) - by);
    board = new GFXcanvas16(bw, bh);
    if (board->getBuffer()) {
        gx = -bx;
        gy = -by;
    } else {  // not enough memory: draw straight onto the screen instead
        Serial.println("Face: not enough memory for smooth drawing (is PSRAM set to OPI PSRAM in Tools?). Drawing directly.");
        delete board;
        board = nullptr;
    }
#endif
    Serial.printf("Face: screen is %d wide and %d tall (rotation %d)\n", W, H, TFT_ROTATION & 3);
    faceSet(FACE_HAPPY);
    xTaskCreatePinnedToCore(faceTask, "face", 6144, nullptr, 1, nullptr, 0);  // start the little artist
    return true;
}
