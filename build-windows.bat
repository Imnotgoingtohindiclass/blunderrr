@echo off
REM ── Local build script for Windows (MSYS2 MinGW64) ────────────────────
REM
REM Prerequisites:
REM   1. Install MSYS2 from https://www.msys2.org
REM   2. Open "MSYS2 MinGW 64-bit" terminal
REM   3. Run:  pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-cmake mingw-w64-x86_64-make mingw-w64-x86_64-pkg-config mingw-w64-x86_64-SDL2 mingw-w64-x86_64-SDL2_image
REM   4. cd into this project folder
REM   5. Run this script from inside MSYS2 MinGW 64-bit
echo ^> Building...
cmake -B build -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release
cmake --build build

echo.
echo Build successful! Binary is at: build\chess-bot.exe
echo    Run it with:  cd build ^&^& chess-bot.exe
pause
