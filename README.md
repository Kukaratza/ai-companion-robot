# 🤖 AI Companion Robot

A desktop talking robot you build yourself. Press a button (or say its name), ask anything, and an **ESP32-S3** streams your voice to **Google's Gemini Live API** and plays the spoken answer. It has a colour face on a big screen that blinks, listens, thinks and talks.

Made for kids around 10 years old and their grown-up helpers, using parts from a typical hobby kit.

## 🌐 Interactive guides (open in your browser)

| Guide | For | Link |
|---|---|---|
| **Build Your Own Talking Robot Friend** | Kids | [guide-kids.html](https://kukaratza.github.io/ai-companion-robot/guide-kids.html) |
| **Breadboard Buddy** (animated wiring lesson) | Kids | [guide-breadboard.html](https://kukaratza.github.io/ai-companion-robot/guide-breadboard.html) |
| **Technical guide** | Grown-ups | [guide.html](https://kukaratza.github.io/ai-companion-robot/guide.html) |

Start page: https://kukaratza.github.io/ai-companion-robot/

## 🧰 Parts

ESP32-S3 (N16R8) · INMP441 I2S microphone · MAX98357 I2S amplifier + 3 W speaker · 2.4" colour TFT screen (ILI9341, SPI) · push button · breadboard (Makeronics BB-102, [link](https://www.amazon.com/dp/B07Q34YND5)) + jumper wires.

## 📁 What's in here

| Folder / file | What it is |
|---|---|
| `arduino/AI_Companion` | The full robot, ready to open in **Arduino IDE 2** |
| `arduino/Face_Demo` | Step 1: just the face, no mic/speaker/WiFi needed |
| `arduino/TFT_Test` | Tests the colour screen on its own (colour bars, a blinking face, and the 4 rotations) |
| `arduino/Button_Test` | Helper that finds which two wires of the push button are the switch (C and NO) |
| `src/`, `include/`, `platformio.ini` | Same code as a PlatformIO project (source of truth) |
| `guide-*.html`, `kids-theme.css`, `images/` | The interactive website (`index.html` just opens the Robot Guide) |
| `tools/` | Scripts that regenerate the Arduino sketches and embed the code in the kids guide |
| `PLAN.md` | Design notes and decisions |

After changing code in `src/` or `include/`:

```sh
sh tools/make_arduino_sketches.sh
python3 tools/embed_code_in_kids_guide.py
```

## 🚀 Quick start

1. (Works on macOS with an Apple chip or an Intel chip, and on Windows. The kids guide has a "What computer do you have?" picker that changes the steps to match.) Install **Arduino IDE 2** and the **esp32 by Espressif Systems** board package (version 3.x). Install the libraries **WebSockets** (Markus Sattler), **Adafruit ILI9341** and **Adafruit GFX Library**.
2. Open `arduino/Face_Demo/Face_Demo.ino` first and wire up the colour screen to see the face (`arduino/TFT_Test` tests the screen on its own).
3. For the full robot open `arduino/AI_Companion/AI_Companion.ino`, edit the `secrets.h` tab with your WiFi and a [Gemini API key](https://aistudio.google.com/apikey), choose the Tools settings from the guide, and upload.

The kids guide has every step, with screenshots-style walk-throughs and the exact Tools menu settings.

## ⚠️ Status: please read

- **The firmware has not been compiled or run on real hardware yet.** It was written and checked by reading and with stand-in libraries only. Expect to fix some compile errors or pin mistakes on the first build, and please open an issue or a pull request if you do.
- The ESP32-S3 pin layout is assumed to match the common *ESP32-S3-DevKitC-1*. On a breadboard only ONE row of the board's pins can be wired (the row next to the free holes). Breadboard Buddy puts the brain on strip 1 of the Makeronics BB-102 with the free row at the bottom and the USB ports on the right, so that row reads `3V3, 3V3, RST, 4, 5, … 14, 5V, G`. Turned the other way it reads `G, 5V, 14, 13, … 4, RST, 3V3, 3V3`. The screen, mic and amp go on strip 2. Every power rail of the BB-102 is cut in the middle (after column 30), so keep power wires in columns 1 to 30. Always trust the labels printed on your own board.
- The colour screen only listens (no way to ask it if it is connected), so use `arduino/TFT_Test` if the face stays dark.
- The **wake word** ("Hi ESP", or a typed name) is experimental, off by default, and written from memory of the Arduino `ESP_SR` library.
- The Gemini model name in `config.h` may change. Check Google's current Live API docs.
- **Keep your API key private.** `arduino/AI_Companion/secrets.h` is committed with placeholders only. Never commit a real key. Set a usage limit in Google AI Studio.
- AI can make mistakes. Supervise young builders and their first chats.
- The part photos are for illustration. If you own one and want it removed, open an issue.

## 📄 License

[MIT](LICENSE)
