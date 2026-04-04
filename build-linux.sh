#!/usr/bin/env bash
# ── Local build script for Linux ────────────────────────────────────────
# Requires: sudo apt install gcc cmake pkg-config libsdl2-dev libsdl2-image-dev
set -euo pipefail
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
cd "$SCRIPT_DIR"

echo "► Installing system dependencies (if missing)..."
sudo apt-get update -qq
sudo apt-get install -y gcc cmake pkg-config libsdl2-dev libsdl2-image-dev

echo "► Building..."
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j"$(nproc)"

echo ""
echo "✅ Build successful! Binary is at: build/chess-bot"
echo "   Run it with:  cd build && ./chess-bot"
