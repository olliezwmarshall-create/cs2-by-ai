#!/bin/sh
set -eu
cd "$(dirname "$0")"
mkdir -p dist
"${CXX:-x86_64-w64-mingw32-g++}" -std=c++17 -O2 -Wall -Wextra -static -static-libgcc -static-libstdc++ -mwindows main.cpp -o dist/Breach.exe -lgdi32 -luser32 -lwinmm

mkdir -p dist/assets
cp assets/*.brm assets/*.txt assets/CREDITS.md assets/sources.json dist/assets/
