#!/bin/sh
# Regenerates the Arduino IDE sketches from the PlatformIO sources (src/ + include/).
# Run from the project root after editing any code:  sh tools/make_arduino_sketches.sh
set -e
rm -rf arduino/AI_Companion arduino/Face_Demo arduino/Button_Test arduino/TFT_Test
mkdir -p arduino/AI_Companion arduino/Face_Demo arduino/Button_Test arduino/TFT_Test

# --- full robot ---
cp src/audio_in.cpp src/audio_in.h src/audio_out.cpp src/audio_out.h \
   src/face.cpp src/face.h src/gemini_live.cpp src/gemini_live.h \
   src/wakeword.cpp src/wakeword.h arduino/AI_Companion/
cp include/config.h arduino/AI_Companion/
cp src/main.cpp arduino/AI_Companion/AI_Companion.ino
if [ -f include/secrets.h ]; then cp include/secrets.h arduino/AI_Companion/secrets.h
else cp include/secrets.example.h arduino/AI_Companion/secrets.h; fi

# --- step 1: face only ---
cp src/face.cpp src/face.h arduino/Face_Demo/
cp include/config.h arduino/Face_Demo/
cp src/face_demo.cpp arduino/Face_Demo/Face_Demo.ino
# --- find the button wires ---
cp include/config.h arduino/Button_Test/
cp src/button_test.cpp arduino/Button_Test/Button_Test.ino
# --- try the big colour screen ---
cp src/tft_test.cpp arduino/TFT_Test/TFT_Test.ino
cp include/config.h arduino/TFT_Test/
echo "Done: arduino/AI_Companion, Face_Demo, Button_Test and TFT_Test"
