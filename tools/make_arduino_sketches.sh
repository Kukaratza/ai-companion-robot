#!/bin/sh
# Regenerates the Arduino IDE sketches from the PlatformIO sources (src/ + include/).
# Run from the project root after editing any code:  sh tools/make_arduino_sketches.sh
set -e
rm -rf arduino/AI_Companion arduino/Face_Demo
mkdir -p arduino/AI_Companion arduino/Face_Demo

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
echo "Done: arduino/AI_Companion and arduino/Face_Demo"
