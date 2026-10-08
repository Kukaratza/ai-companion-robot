// BUTTON FINDER: which two wires of the button are C and NO (the switch we use)?
// Upload this, open the Serial Monitor (115200), and follow the steps it prints.
#include <Arduino.h>
#include "config.h"

static int last = -1;  // -1 = we have not looked yet

void setup() {
    Serial.begin(115200);
    delay(800);
    pinMode(PIN_BUTTON, INPUT_PULLUP);  // the pin reads HIGH until it is joined to GND
    Serial.println("=== BUTTON FINDER ===");
    Serial.println("1) Unplug the USB cable. Touch one button wire to the GPIO4 hole and another");
    Serial.println("   wire to the GND rail. 2) Plug the USB cable back in. 3) Push the button.");
    Serial.println("The right pair says PRESSED only WHILE you push, and 'not pressed' when you let go.");
    Serial.println("Nothing changes?  Try a different pair of wires.");
    Serial.println("Says PRESSED all the time?  That pair is C and NC (normally closed). Try C and NO.");
    Serial.println("NEVER connect the LED wires of a 12V, 24V or 220V button to this board.");
}

void loop() {
    int v = digitalRead(PIN_BUTTON);
    if (v != last) {
        last = v;
        Serial.println(v == LOW ? "PRESSED" : "not pressed");
    }
    delay(20);
}
