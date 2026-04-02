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
1. Download blunderrr.dmg
2. Open the .dmg file
3. Drag blunderrr.app to your Applications folder
4. Launch from Launchpad, Spotlight, or Applications

Tip: First launch? If macOS blocks it, right-click the app → Open → Open to bypass Gatekeeper.

### Windows
1. Download ChessBot-Windows.zip
2. Right-click → Extract All... (or use 7-Zip/WinRAR)
3. Open the extracted folder
4. Double-click chess-bot.exe

Tip: SmartScreen warning? Click More info → Run anyway (standard for unsigned indie apps).

### Linux
1. Download ChessBot-Linux.tar.gz
2. Extract: tar -xzf ChessBot-Linux.tar.gz
3. Enter directory: cd ChessBot-Linux
4. Run: ./run.sh or double-click run.sh in your file manager

Tip: Permission denied? Run chmod +x run.sh bin/chess-bot first. Works on Ubuntu 20.04+, Fedora 33+, Arch, Debian, and most modern distros.

### System Requirements
| Platform | Minimum OS | Architecture | Disk Space |
|----------|------------|--------------|------------|
| macOS | 10.15 Catalina | Intel & Apple Silicon | ~50 MB |
| Windows | Windows 10 (64-bit) | x86_64 | ~50 MB |
| Linux | glibc 2.31+ (Ubuntu 20.04+, Fedora 33+, etc.) | x86_64 | ~50 MB |

### Troubleshooting
| Issue | Fix |
|-------|-----|
| Missing pieces / blank board | Ensure the img/ folder is in the same directory as the executable |
| "Opening book not found" in console | The game still works perfectly; komodo.txt is optional |
| macOS "app is damaged and can't be opened" | Run xattr -cr blunderrr.app in Terminal, then relaunch |
| Linux run.sh: Permission denied | Run chmod +x run.sh or launch via ./bin/chess-bot |

### Downloads
Grab the latest release for your platform from the Releases page: http://github.com/imnotgoingtohindiclass/blunderrr/releases

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
- Move history in algebraic notation (e.g., e2e4, Nb1c3, O-O)
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

Edit these constants in bot.c before compiling:

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

The engine supports a simple text-based opening book (komodo.txt).

### File Format
```
<move_sequence> | <weight>
```

- move_sequence: Space-separated algebraic moves from the starting position
- weight: Integer representing move preference (higher = more likely)

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
1. Place komodo.txt in the same directory as the executable
2. The engine automatically loads it on startup
3. Console will show: opening book loaded: X lines

### Creating Your Own Book
1. Export games from chess databases in coordinate notation
2. Format as move1 move2 move3 | weight
3. Save as komodo.txt alongside the executable

---

## Code Structure

```
chess/
├── bot.c                 # Main game loop, rendering, input handling
├── img/                   # Piece texture images (PNG format)
│   ├── wP.png, wR.png, ... # White pieces
│   └── bP.png, bR.png, ... # Black pieces
├── komodo.txt             # Opening book file (optional)
├── README.md              # This file
└── Makefile               # Build configuration (source builds only)
```

### Key Functions
| Function | Purpose |
|----------|---------|
| isValidMove() | Validates move legality including special rules |
| generateMoves() | Generates all pseudo-legal moves for a player |
| isSquareAttacked() / isKingInCheck() | Check detection logic |
| executeMove() | Applies move to board with side effects |
| searchMakeMove() | Silent move application for search (no UI updates) |
| alphaBeta() | Recursive minimax with alpha-beta pruning |
| evaluateBoard() | Positional + material evaluation from White's perspective |
| loadOpeningBook() / getBookMove() | Opening book integration |
| renderPromotionChoice() / handlePromotionClick() | Pawn promotion UI |

---

## Evaluation Function Details

The board evaluation (evaluateBoard()) returns a score in centipawns from White's perspective:

### Components
1. Material Balance (+/- piece values)
2. Piece-Square Tables: Positional bonuses based on piece location
3. Mobility Bonus: +3 centipawns per legal move advantage
4. Endgame Detection: Switches king table when non-pawn material <= 1300 per side

### Example Scores
| Position | Approx. Score |
|----------|--------------|
| Equal material, balanced position | 0 to ±50 |
| One pawn advantage | +100 |
| Knight for two pawns | ~+20 to +40 |
| Checkmate | ±999000+ |

---

## Known Limitations

1. Search Depth: Fixed at 3 ply; deeper searches may cause noticeable delays
2. No Transposition Table: Repeated positions are re-evaluated
3. No Quiescence Search: Tactical sequences may be mis-evaluated at leaf nodes
4. Simple Move Ordering: Only MVV-LVA heuristic
5. No Draw Detection: Threefold repetition and 50-move rule not implemented
6. Single-Threaded: UI may freeze briefly during AI calculations
7. Pre-built Apps: Configuration changes require building from source

---

## Building from Source

Only needed if you want to modify the code or configuration.

### Requirements
- C Compiler: GCC, Clang, or MSVC with C99 support
- SDL2 Library: Simple DirectMedia Layer 2 (https://www.libsdl.org/)
- SDL2_image: For PNG texture loading

### Installation (Linux/Ubuntu)
```bash
sudo apt update
sudo apt install libsdl2-dev libsdl2-image-dev build-essential
```

### Installation (macOS - Homebrew)
```bash
brew install sdl2 sdl2_image
```

### Build Commands

#### Linux/macOS (GCC/Clang)
```bash
gcc -o chess bot.c -lSDL2 -lSDL2_image -lm -std=c99 -O2
```

#### Windows (MinGW)
```bash
gcc -o chess.exe bot.c -lmingw32 -lSDL2main -lSDL2 -lSDL2_image -lm
```

#### Using Makefile
```makefile
CC = gcc
CFLAGS = -std=c99 -O2 -Wall -Wextra
LDFLAGS = -lSDL2 -lSDL2_image -lm

TARGET = chess
SRC = bot.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

clean:
	rm -f $(TARGET)

.PHONY: all clean
```

```bash
make
```

### Required Assets for Source Builds
Ensure the img/ folder with piece textures is in the executable directory:
```
img/
├── wP.png, wR.png, wN.png, wB.png, wQ.png, wK.png
└── bP.png, bR.png, bN.png, bB.png, bQ.png, bK.png
```

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

### Pull Request Guidelines
1. Fork the repository
2. Create a feature branch (git checkout -b feature/amazing-feature)
3. Commit changes (git commit -m 'Add amazing feature')
4. Push to branch (git push origin feature/amazing-feature)
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

---

Note: This engine is designed for educational purposes and casual play. For competitive analysis, consider stronger engines like Stockfish or Leela Chess Zero.

Happy coding and good luck on the 64 squares!
