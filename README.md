# 🤖 AI Companion Robot

A desktop talking robot you build yourself. Press a button (or say its name), ask anything, and an **ESP32-S3** streams your voice to **Google's Gemini Live API** and plays the spoken answer. It has a face on a tiny OLED screen that blinks, listens, thinks and talks.

Made for kids around 10 years old and their grown-up helpers, using parts from a typical hobby kit.

## 🌐 Interactive guides (open in your browser)

| Guide | For | Link |
|---|---|---|
| **Build Your Own Talking Robot Friend** | Kids | [guide-kids.html](https://kukaratza.github.io/ai-companion-robot/guide-kids.html) |
| **Breadboard Buddy** (animated wiring lesson) | Kids | [guide-breadboard.html](https://kukaratza.github.io/ai-companion-robot/guide-breadboard.html) |
| **Technical guide** | Grown-ups | [guide.html](https://kukaratza.github.io/ai-companion-robot/guide.html) |

Start page: https://kukaratza.github.io/ai-companion-robot/

## 🧰 Parts

ESP32-S3 (N16R8) · INMP441 I2S microphone · MAX98357 I2S amplifier + 3 W speaker · 0.96" SSD1306 OLED (I2C) · push button · breadboard + jumper wires.

## 📁 What's in here

| Folder / file | What it is |
|---|---|
| `arduino/AI_Companion` | The full robot, ready to open in **Arduino IDE 2** |
| `arduino/Face_Demo` | Step 1: just the face, no mic/speaker/WiFi needed |
| `arduino/Button_Test` | Helper that finds which two wires of the push button are the switch (C and NO) |
| `src/`, `include/`, `platformio.ini` | Same code as a PlatformIO project (source of truth) |
| `guide-*.html`, `kids-theme.css`, `index.html`, `images/` | The interactive website |
| `tools/` | Scripts that regenerate the Arduino sketches and embed the code in the kids guide |
| `PLAN.md` | Design notes and decisions |

After changing code in `src/` or `include/`:

```sh
sh tools/make_arduino_sketches.sh
python3 tools/embed_code_in_kids_guide.py
```

## 🚀 Quick start

1. Install **Arduino IDE 2** and the **esp32 by Espressif Systems** board package (version 3.x). Install the libraries **WebSockets** (Markus Sattler), **Adafruit SSD1306** and **Adafruit GFX Library**.
2. Open `arduino/Face_Demo/Face_Demo.ino` first and wire up the OLED to see the face.
3. For the full robot open `arduino/AI_Companion/AI_Companion.ino`, edit the `secrets.h` tab with your WiFi and a [Gemini API key](https://aistudio.google.com/apikey), choose the Tools settings from the guide, and upload.

The kids guide has every step, with screenshots-style walk-throughs and the exact Tools menu settings.

## ⚠️ Status: please read

- **The firmware has not been compiled or run on real hardware yet.** It was written and checked by reading and with stand-in libraries only. Expect to fix some compile errors or pin mistakes on the first build, and please open an issue or a pull request if you do.
- The ESP32-S3 pin layout is assumed to match the common *ESP32-S3-DevKitC-1*. Always trust the labels printed on your own board.
- The **wake word** ("Hi ESP", or a typed name) is experimental, off by default, and written from memory of the Arduino `ESP_SR` library.
- The Gemini model name in `config.h` may change. Check Google's current Live API docs.
- **Keep your API key private.** `arduino/AI_Companion/secrets.h` is committed with placeholders only. Never commit a real key. Set a usage limit in Google AI Studio.
- AI can make mistakes. Supervise young builders and their first chats.
- The part photos are for illustration. If you own one and want it removed, open an issue.

## 📄 License

[MIT](LICENSE)
