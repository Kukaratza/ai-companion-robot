// THE FACE: draws the robot's eyes and mouth on the big colour screen.
#include "face.h"
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include "config.h"

// Make a colour from amounts of red, green and blue (0 to 255 each).
#define RGB565(r, g, b) ((((r) & 0xF8) << 8) | (((g) & 0xFC) << 3) | ((b) >> 3))
#define BLACK 0x0000

// The screen lies on its side: 320 dots across and 240 dots down. (0, 0) is the top-left corner.
static const int W = 320;
static const int BAND = 40;             // the coloured strip at the top that shows a word
static const int FX = 20, FY = 48;      // where the face picture starts
static const int FW = 280, FH = 184;    // how big the face picture is

// One word and one colour for every mood. The order matches the list in face.h.
static const char *LABEL[] = {"READY", "HELLO!", "LISTENING", "THINKING", "TALKING", "ZZZ"};
static const uint16_t COLOUR[] = {
    RGB565(0, 220, 255),    // READY: cyan eyes
    RGB565(255, 120, 190),  // HELLO: pink eyes
    RGB565(90, 255, 140),   // LISTENING: green eyes
    RGB565(190, 140, 255),  // THINKING: purple eyes
    RGB565(255, 220, 60),   // TALKING: yellow eyes
    RGB565(110, 150, 255),  // ZZZ: blue eyes
};

static Adafruit_ILI9341 tft(&SPI, PIN_TFT_DC, PIN_TFT_CS, PIN_TFT_RST);  // "tft" is our name for the screen
static GFXcanvas16 canvas(FW, FH);  // a hidden drawing board: we draw there, then show it all at once (no flicker!)
static volatile FaceMood mood = FACE_IDLE;  // which face are we making right now?
static volatile uint32_t moodSince = 0;     // when did this face start?

// Other files call this to change the face, like faceSet(FACE_HAPPY).
void faceSet(FaceMood m) {
    if (m != mood) {
        mood = m;
        moodSince = millis();  // millis() = how many milliseconds since the robot woke up
    }
}

// Draw ONE eye: a rounded rectangle. cx, cy = its centre. w = width, h = height.
static void eye(int cx, int cy, int w, int h, uint16_t colour) {
    if (h < 6) h = 6;  // never thinner than a line
    canvas.fillRoundRect(cx - w / 2, cy - h / 2, w, h, min(24, h / 2), colour);
}

// Colour the strip at the top and write the mood's word on it.
static void header(FaceMood m) {
    tft.fillRect(0, 0, W, BAND, COLOUR[m]);
    tft.setTextSize(3);
    tft.setTextColor(BLACK, COLOUR[m]);
    tft.setCursor(12, 9);
    tft.print(LABEL[m]);
}

// This "task" runs on its own, all the time, like a little artist inside the robot.
static void faceTask(void *) {
    const int LX = 70, RX = 210, CY = 78;  // where the LEFT eye, RIGHT eye and eye height are
    const int ew = 76, eh = 84;            // eye width and height
    const float blinkScale[] = {0.7f, 0.35f, 0.1f, 0.35f, 0.7f};  // how closed the eye is, frame by frame
    int blinkFrame = -1;  // -1 means "not blinking right now"
    uint32_t nextBlink = millis() + 2500, nextLook = millis() + 3000;
    int lookX = 0, lookY = 0;  // how far the eyes look left/right and up/down
    int shownMood = -1;        // which mood's top strip is on the screen

    for (;;) {  // forever...
        uint32_t now = millis();
        FaceMood m = mood;

        // HELLO only lasts 2.5 seconds, then we go back to READY.
        if (m == FACE_HAPPY && now - moodSince > 2500) {
            faceSet(FACE_IDLE);
            m = FACE_IDLE;
        }
        if (shownMood != (int)m) {  // new mood? Change the strip at the top.
            header(m);
            shownMood = (int)m;
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

        canvas.fillScreen(BLACK);  // wipe the hidden board, then draw the new picture on it
        const uint16_t c = COLOUR[m];
        switch (m) {  // pick the drawing that matches the mood
            case FACE_IDLE:  // waiting: open eyes that blink and look around
                eye(LX + lookX, CY + lookY, ew, eh * bs, c);
                eye(RX + lookX, CY + lookY, ew, eh * bs, c);
                break;
            case FACE_HAPPY:  // happy: ^ ^ eyes, a smile and rosy cheeks
                for (int cx : {LX, RX}) {
                    canvas.fillRoundRect(cx - 36, CY - 28, 72, 60, 26, c);
                    canvas.fillCircle(cx, CY + 32, 40, BLACK);  // cut away the bottom to make a ^
                    canvas.fillCircle(cx + (cx < 140 ? -22 : 22), 128, 12, RGB565(255, 90, 120));  // cheek
                }
                canvas.fillCircle(140, 128, 30, c);       // a smile is a half moon:
                canvas.fillCircle(140, 112, 30, BLACK);   // a circle with the top bitten off
                break;
            case FACE_LISTEN: {  // listening: big round eyes and pulsing dots
                eye(LX, CY - 6, ew + 8, (eh + 20) * bs, c);
                eye(RX, CY - 6, ew + 8, (eh + 20) * bs, c);
                int ph = (now / 200) % 3;  // which dot is big right now?
                for (int i = 0; i < 3; i++) canvas.fillCircle(100 + i * 40, 160, (i == ph) ? 10 : 6, c);
                break;
            }
            case FACE_THINK: {  // thinking: eyes swing side to side and "..." appear
                int sway = (int)(22 * sin(now / 350.0));  // sin() makes a smooth back-and-forth
                eye(LX + sway, CY - 8, ew, eh * bs, c);
                eye(RX + sway, CY - 8, ew, eh * bs, c);
                int dots = (now / 350) % 4;  // 0, 1, 2 or 3 dots
                for (int i = 0; i < dots; i++) canvas.fillCircle(100 + i * 40, 160, 9, c);
                break;
            }
            case FACE_SPEAK: {  // talking: a mouth that opens and closes
                eye(LX, CY - 10, ew, eh * bs, c);
                eye(RX, CY - 10, ew, eh * bs, c);
                int mh = 12 + (int)(14 + 14 * sin(now / 90.0 + random(0, 3)));  // mouth height wobbles
                canvas.fillRoundRect(100, 140, 80, mh, 10, RGB565(255, 120, 120));
                break;
            }
            case FACE_SLEEP:  // sleeping: thin closed eyes and a floating z
                canvas.fillRoundRect(LX - 36, CY + 4, 72, 8, 4, c);
                canvas.fillRoundRect(RX - 36, CY + 4, 72, 8, 4, c);
                canvas.setTextColor(c);
                canvas.setTextSize(5);
                canvas.setCursor(215 + (int)(8 * sin(now / 400.0)), 20);
                canvas.print("z");
                break;
        }
        // Show the whole hidden picture on the screen in ONE go: open a window on the
        // screen (setAddrWindow), then pour all the dots through it (writePixels).
        tft.startWrite();
        tft.setAddrWindow(FX, FY, FW, FH);
        tft.writePixels(canvas.getBuffer(), FW * FH);
        tft.endWrite();
        vTaskDelay(pdMS_TO_TICKS(30));
    }
}

// Get the screen ready. The screen only LISTENS, so we cannot ask it "are you there?".
// If nothing shows up, check the wires.
bool faceBegin() {
    if (!canvas.getBuffer()) {
        Serial.println("Face: not enough memory for the picture (is PSRAM set to OPI PSRAM in Tools?)");
        return false;
    }
    SPI.begin(PIN_TFT_SCK, -1, PIN_TFT_MOSI, -1);  // no MISO wire: the screen only listens
    tft.begin(TFT_SPI_HZ);
    // The face is made for the WIDE screen (320 across). Rotations 0 and 2 are the TALL
    // screen: the right side of the face would run off the edge. So only 1 and 3 are allowed.
    uint8_t rotation = TFT_ROTATION;
    if (rotation % 2 == 0) {
        Serial.println("Face: TFT_ROTATION must be 1 or 3 (the screen on its side). Using 1.");
        rotation = 1;
    }
    tft.setRotation(rotation);
    tft.fillScreen(BLACK);
    faceSet(FACE_HAPPY);
    xTaskCreatePinnedToCore(faceTask, "face", 6144, nullptr, 1, nullptr, 0);  // start the little artist
    return true;
}
