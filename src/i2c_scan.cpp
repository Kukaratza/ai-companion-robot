// I2C SCANNER: finds the face screen, and tells us if its two data wires are right.
// Upload this, open the Serial Monitor (115200), and read what it says.
#include <Arduino.h>
#include <Wire.h>

// Look for anything connected to these two pins. Prints the "address" of what it finds.
static void scan(int sda, int scl) {
    Wire.begin(sda, scl);
    Wire.setClock(100000);
    int found = 0;
    for (int addr = 1; addr < 127; addr++) {
        Wire.beginTransmission(addr);
        if (Wire.endTransmission() == 0) {  // 0 means "someone answered!"
            Serial.printf("   FOUND something at address 0x%02X\n", addr);
            found++;
        }
    }
    if (!found) Serial.println("   nothing found");
    Wire.end();
}

void setup() {
    Serial.begin(115200);
    delay(2500);  // give the Serial Monitor time to open
    Serial.println("=== I2C SCANNER ===");
}

void loop() {
    Serial.println("Try SDA = pin 18 and SCL = pin 17 (the guide's wiring):");
    scan(18, 17);
    Serial.println("Try them swapped, SDA = pin 17 and SCL = pin 18:");
    scan(17, 18);
    Serial.println("If the screen shows 0x3C (or 0x3D) it is connected. Wait for the next round...\n");
    delay(3000);
}
