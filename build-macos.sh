#!/usr/bin/env bash
# ── Local build script for macOS ────────────────────────────────────────
# Requires: Homebrew (https://brew.sh)
set -euo pipefail
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
cd "$SCRIPT_DIR"

echo "► Installing dependencies via Homebrew (if missing)..."
brew install cmake sdl2 sdl2_image 2>/dev/null || true

echo "► Building..."
cmake -B build -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_PREFIX_PATH="$(brew --prefix)"
cmake --build build -j"$(sysctl -n hw.ncpu)"

echo ""
echo "✅ Build successful! Binary is at: build/chess-bot"
echo "   Run it with:  cd build && ./chess-bot"
