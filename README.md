# blunderrr

A feature-rich chess engine with an AI opponent featuring alpha-beta pruning, opening book support, and full chess rule implementation. Pre-built applications available for instant play!

Chess - Standard Rules | AI - AlphaBeta Search | Cross-Platform - Windows | macOS | Linux

---

## Table of Contents

- [Features](#-features)
- [Quick Start - No Installation Required](#-quick-start---no-installation-required)
- [Usage](#-usage)
- [Configuration](#-configuration)
- [Opening Book Format](#-opening-book-format)
- [Code Structure](#-code-structure)
- [Evaluation Function](#-evaluation-function)
- [Known Limitations](#-known-limitations)
- [Building from Source](#-building-from-source)
- [Contributing](#-contributing)
- [License](#-license)

---

## Features

### Chess Rules
- Full piece movement (Pawns, Knights, Bishops, Rooks, Queens, Kings)
- Castling (kingside & queenside) with proper rights tracking
- En passant captures
- Pawn promotion with interactive piece selection (Queen, Rook, Knight, Bishop)
- Check and checkmate detection
- Stalemate detection

### AI Opponent
- Alpha-Beta Pruning search algorithm with configurable depth (SEARCH_DEPTH = 3)
- Opening Book Support: Load PGN-style opening lines for stronger early-game play
- Move Ordering: MVV-LVA heuristic for faster pruning
- Evaluation Function includes material, piece-square tables, mobility, and endgame awareness

### User Interface
- Drag-and-drop piece movement with smooth SDL2 rendering
- Visual promotion choice panel when pawns reach the 8th rank
- Algebraic move notation displayed in console
- Check/checkmate status messages

### Game Modes
- Human vs Bot (default): Play as White against the AI
- Human vs Human: Disable bot mode for local two-player games

---

## Installation

No dependencies, no package managers, no terminal setup. Just download, open, and play.

### macOS

1. Download `chess-bot-macos-arm64.zip` from the [Releases](https://github.com/imnotgoingtohindiclass/blunderrr/releases) page
2. Double-click the `.zip` to extract it (macOS does this automatically)
3. Open the extracted folder
4. Double-click `chess-bot` to launch

> **Gatekeeper warning?** Right-click the app → Open → Open. If macOS says the app "is damaged," run `xattr -cr chess-bot` in Terminal from the same folder, then relaunch.

### Windows

1. Download `chess-bot-windows-x86_64.zip` from the [Releases](https://github.com/imnotgoingtohindiclass/blunderrr/releases) page
2. Right-click → Extract All... (or use 7-Zip / WinRAR)
3. Open the extracted folder
4. Double-click `chess-bot.exe`

> **SmartScreen warning?** Click "More info" → "Run anyway." This is standard for unsigned indie apps — Windows flags anything without a paid code-signing certificate.

### Linux

1. Download `chess-bot-linux-x86_64.tar.gz` from the [Releases](https://github.com/imnotgoingtohindiclass/blunderrr/releases) page
2. Extract: `tar -xzf chess-bot-linux-x86_64.tar.gz`
3. Enter the directory: `cd chess-bot-linux-x86_64`
4. Run: `./chess-bot`

> **Permission denied?** Run `chmod +x chess-bot` first. Works on Ubuntu 20.04+, Fedora 33+, Arch, Debian, and most modern distros with glibc 2.31+.
>
> **Missing SDL2 at runtime?** On some distros the SDL2 runtime libraries aren't pre-installed. Install them with:
> ```bash
> # Ubuntu / Debian
> sudo apt install libsdl2-2.0-0 libsdl2-image-2.0-0
>
> # Fedora
> sudo dnf install SDL2 SDL2_image
>
> # Arch
> sudo pacman -S sdl2 sdl2_image
> ```

### System Requirements

| Platform | Minimum OS | Architecture | Disk Space |
|----------|------------|--------------|------------|
| macOS | 10.15 Catalina | Apple Silicon (ARM64) | ~50 MB |
| Windows | Windows 10 (64-bit) | x86_64 | ~50 MB |
| Linux | glibc 2.31+ (Ubuntu 20.04+) | x86_64 | ~50 MB |

### Troubleshooting

| Issue | Fix |
|-------|-----|
| Missing pieces / blank board | Ensure the `img/` folder is in the same directory as the executable |
| "Opening book not found" in console | The game still works perfectly; `komodo.txt` is optional |
| macOS "app is damaged and can't be opened" | Run `xattr -cr chess-bot` in Terminal, then relaunch |
| Linux `./chess-bot`: Permission denied | Run `chmod +x chess-bot` first |
| Linux: error while loading shared libraries | Install SDL2 runtime (see Linux install step 4 above) |
| Windows SmartScreen blocks the exe | Click "More info" → "Run anyway" |

### Downloads

Grab the latest release for your platform from the Releases page: https://github.com/imnotgoingtohindiclass/blunderrr/releases

---

## Usage

### Controls

| Action | Input |
|--------|-------|
| Select piece | Left-click on your piece |
| Move piece | Drag to destination square and release |
| Promote pawn | Click desired piece (Q/R/N/B) in promotion panel |
| Quit game | Close window or press Alt+F4 / Cmd+Q |

### Console Output

- Move history in algebraic notation (e.g., `e2e4`, `Nb1c3`, `O-O`)
- Check/checkmate announcements
- AI evaluation scores during bot moves
- Opening book match notifications

### First Run Tips

1. White moves first by default (human player)
2. The bot responds automatically after your move
3. Watch the terminal for move history and game status
4. Pawn promotion: when your pawn reaches the end, click Q/R/N/B to choose

---

## Configuration

Configuration requires building from source. Pre-built apps use default settings.

Edit these constants in `chess.c` before compiling:

```c
#define SCREEN_WIDTH 640          // Window width (must be divisible by 8)
#define SCREEN_HEIGHT 640         // Window height
#define SQUARE_SIZE (SCREEN_WIDTH / 8)  // Calculated automatically
#define SEARCH_DEPTH 3            // AI search depth (higher = stronger but slower)
#define INF 999999                // Internal infinity value for alpha-beta
```

### Game Mode Settings

```c
bool vsBot = true;           // Set to false for human vs human
char humanPlayer = 'w';      // 'w' for White, 'b' for Black
```

### Piece Values (for evaluation)

```c
#define PAWN_VAL   100
#define KNIGHT_VAL 320
#define BISHOP_VAL 330
#define ROOK_VAL   500
#define QUEEN_VAL  900
#define KING_VAL   20000
```

---

## Opening Book Format

The engine supports a simple text-based opening book (`komodo.txt`).

### File Format

```
<move_sequence> | <weight>
```

- `move_sequence`: Space-separated algebraic moves from the starting position
- `weight`: Integer representing move preference (higher = more likely)

### Example komodo.txt

```
e2e4 | 100
e2e4 e7e5 | 80
e2e4 c7c5 | 70
e2e4 e7e5 g1f3 | 60
d2d4 | 90
d2d4 d7d5 | 75
```

### How to Use

1. Place `komodo.txt` in the same directory as the executable
2. The engine automatically loads it on startup
3. Console will show: `opening book loaded: X lines`

### Creating Your Own Book

1. Export games from chess databases in coordinate notation
2. Format as `move1 move2 move3 | weight`
3. Save as `komodo.txt` alongside the executable

---

## Code Structure

```
blunderrr/
├── chess.c                # Main game loop, AI, rendering, input handling
├── CMakeLists.txt         # Cross-platform CMake build configuration
├── img/                    # Piece texture images (PNG format)
│   ├── wP.png, wR.png, wN.png, wB.png, wQ.png, wK.png   # White pieces
│   └── bP.png, bR.png, bN.png, bB.png, bQ.png, bK.png   # Black pieces
├── komodo.txt              # Opening book file (optional)
├── build-linux.sh          # One-click Linux build script
├── build-macos.sh          # One-click macOS build script
├── build-windows.bat       # One-click Windows build script (MSYS2)
├── .github/
│   └── workflows/
│       └── build-release.yml   # CI/CD: auto-build + GitHub Releases
└── README.md               # This file
```

### Key Functions

| Function | Purpose |
|----------|---------|
| `isValidMove()` | Validates move legality including special rules |
| `generateMoves()` | Generates all pseudo-legal moves for a player |
| `isSquareAttacked()` / `isKingInCheck()` | Check detection logic |
| `executeMove()` | Applies move to board with side effects |
| `searchMakeMove()` | Silent move application for search (no UI updates) |
| `alphaBeta()` | Recursive minimax with alpha-beta pruning |
| `evaluateBoard()` | Positional + material evaluation from White's perspective |
| `loadOpeningBook()` / `getBookMove()` | Opening book integration |
| `renderPromotionChoice()` / `handlePromotionClick()` | Pawn promotion UI |

---

## Evaluation Function Details

The board evaluation (`evaluateBoard()`) returns a score in centipawns from White's perspective.

### Components

1. **Material Balance** (+/- piece values)
2. **Piece-Square Tables**: Positional bonuses based on piece location
3. **Mobility Bonus**: +3 centipawns per legal move advantage
4. **Endgame Detection**: Switches king table when non-pawn material <= 1300 per side

### Example Scores

| Position | Approx. Score |
|----------|--------------|
| Equal material, balanced position | 0 to ±50 |
| One pawn advantage | +100 |
| Knight for two pawns | ~+20 to +40 |
| Checkmate | ±999000+ |

---

## Known Limitations

1. **Search Depth**: Fixed at 3 ply; deeper searches may cause noticeable delays
2. **No Transposition Table**: Repeated positions are re-evaluated
3. **No Quiescence Search**: Tactical sequences may be mis-evaluated at leaf nodes
4. **Simple Move Ordering**: Only MVV-LVA heuristic
5. **No Draw Detection**: Threefold repetition and 50-move rule not implemented
6. **Single-Threaded**: UI may freeze briefly during AI calculations
7. **Pre-built Apps**: Configuration changes require building from source

---

## Building from Source

Only needed if you want to modify the code or configuration. Pre-built binaries are available on the [Releases](https://github.com/imnotgoingtohindiclass/blunderrr/releases) page.

### Prerequisites

| Platform | Dependencies |
|----------|-------------|
| **Linux (Ubuntu/Debian)** | `sudo apt install gcc cmake pkg-config libsdl2-dev libsdl2-image-dev` |
| **macOS** | `brew install cmake sdl2 sdl2_image` |
| **Windows** | [MSYS2 MinGW64](https://www.msys2.org) — install GCC + CMake + SDL2 via `pacman` (see below) |

### Quick Build (Scripts)

```bash
# Linux
chmod +x build-linux.sh && ./build-linux.sh

# macOS
chmod +x build-macos.sh && ./build-macos.sh

# Windows — open "MSYS2 MinGW 64-bit" terminal, then:
build-windows.bat
```

### Manual CMake Build

```bash
# 1. Install dependencies (see table above)

# 2. Configure
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release

# 3. Build
cmake --build . -j$(nproc)    # Linux / macOS
cmake --build .                # Windows (MSYS2)

# 4. Run
./chess-bot        # Linux / macOS
chess-bot.exe      # Windows
```

The `img/` folder and `komodo.txt` are automatically copied next to the binary at build time.

### Windows MSYS2 Setup (Detailed)

If you don't have MSYS2 installed yet:

1. Install MSYS2 from https://www.msys2.org
2. Open the **MSYS2 MinGW 64-bit** shortcut (not the regular MSYS2 terminal)
3. Install build tools and SDL2:
   ```bash
   pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-cmake mingw-w64-x86_64-make mingw-w64-x86_64-pkg-config mingw-w64-x86_64-SDL2 mingw-w64-x86_64-SDL2_image
   ```
4. Navigate to the project folder and build as shown above

### Required Assets for Source Builds

Ensure the `img/` folder with piece textures exists in the project root:

```
img/
├── wP.png, wR.png, wN.png, wB.png, wQ.png, wK.png
└── bP.png, bR.png, bN.png, bB.png, bQ.png, bK.png
```

---

## Automated Releases (GitHub Actions)

The included CI/CD workflow (`.github/workflows/build-release.yml`) handles everything automatically:

- Every **push to `main`** builds on all 3 platforms (smoke test)
- Every **version tag** (e.g. `v1.0.0`) creates a GitHub Release with downloadable binaries

### Creating a New Release

```bash
git tag v1.0.0
git push origin v1.0.0
```

GitHub Actions will build for Windows, Linux, and macOS, then upload the artifacts to the release page. No manual packaging required.

---

## Contributing

Contributions are welcome! Areas for improvement:

- Add iterative deepening with time management
- Implement transposition table (Zobrist hashing)
- Add quiescence search to reduce horizon effect
- Support UCI protocol for GUI integration
- Add sound effects for moves/captures/check
- Implement move undo functionality
- Add game save/load (PGN format)
- Create a macOS `.app` bundle for proper Launchpad integration
- Add an x86_64 macOS build target

### Pull Request Guidelines

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Commit changes (`git commit -m 'Add amazing feature'`)
4. Push to branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

---

## License

This project is open source and available under the MIT License.

```
MIT License

Copyright (c) 2026 blunderrr Chess Engine Project

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
```

---

## Acknowledgements

- SDL2 Library (https://www.libsdl.org/) - Cross-platform multimedia framework
- Piece-square tables inspired by Chess Programming Wiki (https://www.chessprogramming.org/)
- Opening book format adapted from standard PGN conventions
- Alpha-beta pruning implementation based on classic minimax algorithms
