// THE FACE: draws the robot's eyes and mouth on the little screen.
#include "face.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "config.h"

// The screen is 128 dots wide and 64 dots tall. The dot at the top-left is (0, 0).
// Some screens have two colours: the top 16 rows are one colour (the "band"),
// the rest is blue. We use the band for a word like READY, and draw the face below.
static const int BAND = 16;

static Adafruit_SSD1306 oled(128, 64, &Wire, -1);  // "oled" is our name for the screen
static volatile FaceMood mood = FACE_IDLE;         // which face are we making right now?
static volatile uint32_t moodSince = 0;            // when did this face start?

// Other files call this to change the face, like faceSet(FACE_HAPPY).
void faceSet(FaceMood m) {
    if (m != mood) {
        mood = m;
        moodSince = millis();  // millis() = how many milliseconds since the robot woke up
    }
}

// Draw ONE eye: a rounded rectangle. cx, cy = its centre. w = width, h = height.
static void eye(int cx, int cy, int w, int h) {
    if (h < 3) h = 3;  // never thinner than a line
    oled.fillRoundRect(cx - w / 2, cy - h / 2, w, h, min(9, h / 2), SSD1306_WHITE);
}

// Write a little word in the top band, and draw a line under it.
static void header(const char *s) {
    oled.setTextSize(1);
    oled.setTextColor(SSD1306_WHITE);
    oled.setCursor(2, 4);
    oled.print(s);
    oled.drawFastHLine(0, BAND - 1, 128, SSD1306_WHITE);
}

// This "task" runs on its own, all the time, like a little artist inside the robot.
static void faceTask(void *) {
    const int LX = 36, RX = 92, CY = 37;  // where the LEFT eye, RIGHT eye and eye height are
    const float blinkScale[] = {0.7f, 0.35f, 0.1f, 0.35f, 0.7f};  // how closed the eye is, frame by frame
    int blinkFrame = -1;  // -1 means "not blinking right now"
    uint32_t nextBlink = millis() + 2500, nextLook = millis() + 3000;
    int lookX = 0, lookY = 0;  // how far the eyes look left/right and up/down

    for (;;) {  // forever...
        uint32_t now = millis();
        FaceMood m = mood;

        // HAPPY only lasts 2.5 seconds, then we go back to IDLE.
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
            lookX = random(-8, 9);
            lookY = random(-3, 4);
            nextLook = now + random(1500, 4000);
        }

        oled.clearDisplay();  // wipe the screen, then draw the new picture
        const int w = 30, h = 28;  // eye width and height
        switch (m) {  // pick the drawing that matches the mood
            case FACE_IDLE:  // waiting: open eyes that blink and look around
                header("READY");
                eye(LX + lookX, CY + lookY, w, h * bs);
                eye(RX + lookX, CY + lookY, w, h * bs);
                break;
            case FACE_HAPPY:  // happy: ^ ^ eyes and a smile
                header("HELLO!");
                for (int cx : {LX, RX}) {
                    oled.fillRoundRect(cx - 15, CY - 12, 30, 26, 11, SSD1306_WHITE);
                    oled.fillCircle(cx, CY + 12, 16, SSD1306_BLACK);  // cut away the bottom to make a ^
                }
                oled.drawCircle(64, 47, 10, SSD1306_WHITE);  // a smile is the bottom half of a circle
                oled.fillRect(52, 34, 24, 13, SSD1306_BLACK);
                break;
            case FACE_LISTEN: {  // listening: big eyes and pulsing dots
                header("LISTENING");
                eye(LX, CY - 3, w + 4, (h + 8) * bs);
                eye(RX, CY - 3, w + 4, (h + 8) * bs);
                int ph = (now / 200) % 3;  // which dot is big right now?
                for (int i = 0; i < 3; i++)
                    oled.fillCircle(50 + i * 14, 59, (i == ph) ? 3 : 2, SSD1306_WHITE);
                break;
            }
            case FACE_THINK: {  // thinking: eyes swing side to side and "..." appear
                header("THINKING");
                int sway = (int)(10 * sin(now / 350.0));  // sin() makes a smooth back-and-forth
                eye(LX + sway, CY - 4, w, h * bs);
                eye(RX + sway, CY - 4, w, h * bs);
                int dots = (now / 350) % 4;  // 0, 1, 2 or 3 dots
                for (int i = 0; i < dots; i++) oled.fillCircle(50 + i * 14, 59, 3, SSD1306_WHITE);
                break;
            }
            case FACE_SPEAK: {  // talking: a mouth that opens and closes
                header("TALKING");
                eye(LX, CY - 5, w, h * bs);
                eye(RX, CY - 5, w, h * bs);
                int mh = 4 + (int)(5 + 5 * sin(now / 90.0 + random(0, 3)));  // mouth height wobbles
                oled.fillRoundRect(50, 53, 28, mh, 4, SSD1306_WHITE);
                break;
            }
            case FACE_SLEEP:  // sleeping: thin closed eyes and a floating z
                header("ZZZ");
                oled.fillRoundRect(LX - 15, CY, 30, 4, 2, SSD1306_WHITE);
                oled.fillRoundRect(RX - 15, CY, 30, 4, 2, SSD1306_WHITE);
                oled.setTextSize(2);
                oled.setCursor(100 + (int)(4 * sin(now / 400.0)), 22);
                oled.print("z");
                break;
        }
        oled.display();                 // show the new picture on the screen
        vTaskDelay(pdMS_TO_TICKS(50));  // rest for 50 ms, so we draw about 20 pictures a second
    }
}

// Get the screen ready. Returns false if no screen is found (the robot still works!).
bool faceBegin() {
    Wire.begin(PIN_OLED_SDA, PIN_OLED_SCL);  // connect to the screen's 2 wires
    Wire.setClock(400000);
    if (!oled.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
        Serial.println("No OLED found - running without a face");
        return false;
    }
#if FACE_FLIP
    oled.setRotation(2);  // the screen is mounted upside down, so turn the picture around
#endif
    faceSet(FACE_HAPPY);
    xTaskCreatePinnedToCore(faceTask, "face", 4096, nullptr, 1, nullptr, 0);  // start the little artist
    return true;
}
