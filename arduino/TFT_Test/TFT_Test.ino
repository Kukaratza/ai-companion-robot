// TFT TEST: does the big colour screen (2.4" 240x320, ILI9341) work? Use it when the
// face stays dark. It draws colour bars and a blinking face, then turns the picture a
// quarter turn every few seconds and writes "rotation 0, 1, 2 or 3". Tell config.h
// (TFT_ROTATION) which number looks right side up.
//
// The wires are listed in config.h (PIN_TFT_...) and in Breadboard Buddy.
#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include "config.h"

Adafruit_ILI9341 tft(&SPI, PIN_TFT_DC, PIN_TFT_CS, PIN_TFT_RST);  // "tft" is our name for the screen

// Draw everything from scratch, for one rotation (0 to 3).
static void drawScene(int rotation, bool eyesOpen) {
    tft.setRotation(rotation);
    const int W = tft.width(), H = tft.height();  // these change when the picture turns
    tft.fillScreen(ILI9341_BLACK);

    // five colour bars across the top: red, green, blue, yellow, white
    const uint16_t colours[] = {ILI9341_RED, ILI9341_GREEN, ILI9341_BLUE, ILI9341_YELLOW, ILI9341_WHITE};
    for (int i = 0; i < 5; i++) tft.fillRect(i * (W / 5), 0, W / 5, H / 6, colours[i]);

    // the face
    const int ew = W / 5, eh = H / 3, ey = H * 2 / 5;
    if (eyesOpen) {
        tft.fillRoundRect(W / 5, ey, ew, eh, 20, ILI9341_CYAN);
        tft.fillRoundRect(W * 3 / 5, ey, ew, eh, 20, ILI9341_CYAN);
    } else {  // blink: thin lines
        tft.fillRoundRect(W / 5, ey + eh / 2, ew, 8, 4, ILI9341_CYAN);
        tft.fillRoundRect(W * 3 / 5, ey + eh / 2, ew, 8, 4, ILI9341_CYAN);
    }
    tft.fillRoundRect(W * 2 / 5, ey + eh + 16, W / 5, 12, 6, ILI9341_YELLOW);

    // which rotation is this?
    tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
    tft.setTextSize(2);
    tft.setCursor(8, H - 22);
    tft.print("rotation ");
    tft.print(rotation);
    tft.print("  ");
    tft.print(W);
    tft.print("x");
    tft.print(H);

    // a white frame round the whole picture: if a side of it is missing, that side is cut off
    tft.drawRect(0, 0, W, H, ILI9341_WHITE);
    tft.drawRect(1, 1, W - 2, H - 2, ILI9341_WHITE);
}

void setup() {
    Serial.begin(115200);
    delay(2500);  // give the Serial Monitor time to open
    Serial.println("TFT test: starting");

    SPI.begin(PIN_TFT_SCK, -1, PIN_TFT_MOSI, -1);  // the screen only listens, so no MISO wire
    tft.begin(TFT_SPI_HZ);
    SPI.end();  // the screen library restarts SPI on its own default pins, so set ours again
    SPI.begin(PIN_TFT_SCK, -1, PIN_TFT_MOSI, -1);
    Serial.println("TFT test: if you can see colours, the screen works!");
}

void loop() {
    for (int rotation = 0; rotation < 4; rotation++) {
        Serial.printf("rotation %d\n", rotation);
        drawScene(rotation, true);
        delay(3000);
        drawScene(rotation, false);  // blink
        delay(250);
    }
}
