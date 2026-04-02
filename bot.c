#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <time.h>

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 640
#define SQUARE_SIZE (SCREEN_WIDTH / 8)
#define SEARCH_DEPTH 3
#define INF 999999

// piece textures indexed by ASCII value of the piece character
SDL_Texture* gPieceTextures[128] = {NULL};
SDL_Renderer* gRenderer = NULL;

char board[8][8] = {
    {'r', 'n', 'b', 'q', 'k', 'b', 'n', 'r'},
    {'p', 'p', 'p', 'p', 'p', 'p', 'p', 'p'},
    {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
    {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
    {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
    {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
    {'P', 'P', 'P', 'P', 'P', 'P', 'P', 'P'},
    {'R', 'N', 'B', 'Q', 'K', 'B', 'N', 'R'}
};

char currentPlayer = 'w';

// en passant target square (-1, -1 when not active)
int enPassantX = -1;
int enPassantY = -1;

// castling rights
bool wKingMoved = false, bKingMoved = false;
bool wRookAMoved = false, wRookHMoved = false;
bool bRookAMoved = false, bRookHMoved = false;

// game state
bool gameOver = false;
bool vsBot = true;
char humanPlayer = 'w';

// promotion state
bool awaitingPromotion = false;
int promotionX = -1;
int promotionY = -1;

// game notation
char gameMovesPlayed[4096] = "";

// drag state
bool isDragging = false;
char draggedPiece = ' ';
int dragStartX = -1, dragStartY = -1;
int mouseX = 0, mouseY = 0;

typedef struct {
    char moves[256];
    int weight;
    char nextMove[6];
} BookLine;

BookLine* openingBook = NULL;
int bookSize = 0;

typedef struct {
    int fromX, fromY, toX, toY;
} Move;

typedef struct {
    char board[8][8];
    char currentPlayer;
    int enPassantX, enPassantY;
    bool wKingMoved, bKingMoved;
    bool wRookAMoved, wRookHMoved;
    bool bRookAMoved, bRookHMoved;
} BoardState;

bool isValidMove(char boardState[8][8], char piece, int fromX, int fromY, int toX, int toY, char player);
bool isSquareAttacked(char boardState[8][8], int x, int y, char attackerColor);
bool isKingInCheck(char boardState[8][8], char kingColor);
bool hasLegalMoves(char playerColor);
void checkEndgame();
void renderPromotionChoice();
void handlePromotionClick(int mx, int my);
void moveToAlgebraic(char piece, int fromX, int fromY, int toX, int toY, char* output);
void recordMove(char piece, int fromX, int fromY, int toX, int toY);
void executeMove(Move* move);
void generateMoves(char player, Move* moves, int* moveCount);
void makeBotMove();
bool loadOpeningBook(const char* filename);
bool algebraicToMove(const char* algebraic, Move* move);
bool getBookMove(Move* bookMove);
int evaluateBoard();
int isEndgame();
int countMaterial(char color);
int getMobility(char color);
BoardState saveState();
void restoreState(BoardState* s);
void searchMakeMove(Move* move);
int moveScore(Move* move);
void orderMoves(Move* moves, int moveCount);
int alphaBeta(int depth, int alpha, int beta);

bool isSquareAttacked(char boardState[8][8], int x, int y, char attackerColor) {
    for (int r = 0; r < 8; ++r) {
        for (int c = 0; c < 8; ++c) {
            char piece = boardState[r][c];
            if (
                piece != ' ' &&
                ((attackerColor == 'w' && isupper(piece)) ||
                 (attackerColor == 'b' && islower(piece)))
            ) {
                // pawns have special attack pattern (diagonal only)
                if (toupper(piece) == 'P') {
                    int pdy = y - r;
                    int pdx = abs(x - c);
                    if (attackerColor == 'w' && pdy == -1 && pdx == 1)
                        return true;
                    if (attackerColor == 'b' && pdy ==  1 && pdx == 1)
                        return true;
                } else if (isValidMove(boardState, piece, c, r, x, y, attackerColor)) {
                    return true;
                }
            }
        }
    }
    return false;
}

bool isKingInCheck(char boardState[8][8], char kingColor) {
    char kingChar = (kingColor == 'w') ? 'K' : 'k';
    char attackerColor = (kingColor == 'w') ? 'b' : 'w';
    int kingX = -1, kingY = -1;

    for (int r = 0; r < 8; ++r) {
        for (int c = 0; c < 8; ++c) {
            if (boardState[r][c] == kingChar) {
                kingX = c;
                kingY = r;
                break;
            }
        }
    }

    if (kingX == -1) return false;

    return isSquareAttacked(boardState, kingX, kingY, attackerColor);
}

bool hasLegalMoves(char playerColor) {
    char tempBoard[8][8];
    for (int fromY = 0; fromY < 8; ++fromY) {
        for (int fromX = 0; fromX < 8; ++fromX) {
            char piece = board[fromY][fromX];
            if (
                piece != ' ' &&
                ((playerColor == 'w' && isupper(piece)) ||
                 (playerColor == 'b' && islower(piece)))
            ) {
                for (int toY = 0; toY < 8; ++toY) {
                    for (int toX = 0; toX < 8; ++toX) {
                        if (isValidMove(board, piece, fromX, fromY, toX, toY, playerColor)) {
                            memcpy(tempBoard, board, sizeof(board));
                            tempBoard[toY][toX] = piece;
                            tempBoard[fromY][fromX] = ' ';

                            // handle en passant removal in the simulation
                            if (
                                toupper(piece) == 'P' &&
                                toX == enPassantX &&
                                toY == enPassantY
                            ) {
                                if (playerColor == 'w')
                                    tempBoard[toY + 1][toX] = ' ';
                                else
                                    tempBoard[toY - 1][toX] = ' ';
                            }

                            // handle castling rook in the simulation
                            if (
                                toupper(piece) == 'K' &&
                                abs(toX - fromX) == 2
                            ) {
                                if (toX == 6) {
                                    tempBoard[fromY][5] = tempBoard[fromY][7];
                                    tempBoard[fromY][7] = ' ';
                                } else {
                                    tempBoard[fromY][3] = tempBoard[fromY][0];
                                    tempBoard[fromY][0] = ' ';
                                }
                            }

                            if (!isKingInCheck(tempBoard, playerColor))
                                return true;
                        }
                    }
                }
            }
        }
    }
    return false;
}

void checkEndgame() {
    if (!hasLegalMoves(currentPlayer)) {
        gameOver = true;
        if (isKingInCheck(board, currentPlayer)) {
            printf("checkmate! %s wins!\n", (currentPlayer == 'w') ? "Black" : "White");
        } else {
            printf("stalemate! draw!\n");
        }
    } else if (isKingInCheck(board, currentPlayer)) {
        printf("%s's turn - check!\n", currentPlayer == 'w' ? "White" : "Black");
    } else {
        printf("%s's turn\n", currentPlayer == 'w' ? "White" : "Black");
    }
}

bool isValidMove(char boardState[8][8], char piece, int fromX, int fromY, int toX, int toY, char player) {
    if (
        fromX  <  0  ||
        fromX  >  7  ||
        fromY  <  0  ||
        fromY  >  7  ||
        toX  <  0  ||
        toX  >  7  ||
        toY  <  0  ||
        toY  >  7
    ) return false;

    if (
        fromX == toX &&
        fromY == toY
    ) return false;

    char dest = boardState[toY][toX];
    // cant kill yourself
    if (dest != ' ' &&
        ((isupper(piece) && isupper(dest)) ||
         (islower(piece) && islower(dest))))
        return false;

    int dx = toX - fromX;
    int dy = toY - fromY;

    switch (toupper(piece)) {

    case 'P': {
        bool isWhite = isupper(piece);
        int dir = isWhite ? -1 : 1; // white/black direction
        int startRow = isWhite ? 6 : 1; // double push starting row

        // move forward 1
        if (dx == 0 && dy == dir && dest == ' ')
            return true;

        // move forward 2 at start
        if (dx == 0 && dy == 2 * dir && fromY == startRow &&
            boardState[fromY + dir][fromX] == ' ' && dest == ' ')
            return true;

        // capture diagonally
        if (abs(dx) == 1 && dy == dir && dest != ' ')
            return true;

        // EN PASSANT
        if (
            abs(dx) == 1 &&
            dy == dir &&
            toX == enPassantX &&
            toY == enPassantY
        )
            return true;

        return false;
    }

    case 'N':
        return (abs(dx) == 1 && abs(dy) == 2) ||
               (abs(dx) == 2 && abs(dy) == 1);

    case 'B': {
        if (abs(dx) != abs(dy) || dx == 0) return false;
        int sx = (dx > 0) ? 1 : -1;
        int sy = (dy > 0) ? 1 : -1;
        for (int i = 1; i < abs(dx); ++i)
            if (boardState[fromY + i * sy][fromX + i * sx] != ' ')
                return false;
        return true;
    }

    case 'R': {
        if (dx != 0 && dy != 0) return false;
        if (dx == 0) {
            int sy = (dy > 0) ? 1 : -1;
            for (int i = 1; i < abs(dy); ++i)
                if (boardState[fromY + i * sy][fromX] != ' ')
                    return false;
        } else {
            int sx = (dx > 0) ? 1 : -1;
            for (int i = 1; i < abs(dx); ++i)
                if (boardState[fromY][fromX + i * sx] != ' ')
                    return false;
        }
        return true;
    }

    case 'Q': {
        if (abs(dx) != abs(dy) && dx != 0 && dy != 0) return false;
        int sx = (dx > 0) ? 1 : ((dx < 0) ? -1 : 0);
        int sy = (dy > 0) ? 1 : ((dy < 0) ? -1 : 0);
        int steps = (abs(dx) > abs(dy)) ? abs(dx) : abs(dy);
        for (int i = 1; i < steps; ++i)
            if (boardState[fromY + i * sy][fromX + i * sx] != ' ')
                return false;
        return true;
    }

    case 'K': {
        // normal one-square move
        if (abs(dx) <= 1 && abs(dy) <= 1)
            return true;

        // castling: king moves two squares horizontally
        if (abs(dx) == 2 && dy == 0) {
            // king can't be in check right now
            char tempBoard[8][8];
            memcpy(tempBoard, boardState, sizeof(char) * 8 * 8);
            tempBoard[fromY][fromX] = ' ';
            if (isKingInCheck(tempBoard, player))
                return false;

            if (player == 'w') {
                if (wKingMoved) return false;

                // kingside
                if (dx == 2) {
                    if (
                        wRookHMoved ||
                        boardState[7][5] != ' ' ||
                        boardState[7][6] != ' '
                    ) return false;
                    if (
                        isSquareAttacked(boardState, 5, 7, 'b') ||
                        isSquareAttacked(boardState, 6, 7, 'b')
                    ) return false;
                    return true;
                }
                // queenside
                if (dx == -2) {
                    if (
                        wRookAMoved ||
                        boardState[7][1] != ' ' ||
                        boardState[7][2] != ' ' ||
                        boardState[7][3] != ' '
                    ) return false;
                    if (
                        isSquareAttacked(boardState, 2, 7, 'b') ||
                        isSquareAttacked(boardState, 3, 7, 'b')
                    ) return false;
                    return true;
                }
            } else {
                if (bKingMoved) return false;

                // kingside
                if (dx == 2) {
                    if (
                        bRookHMoved ||
                        boardState[0][5] != ' ' ||
                        boardState[0][6] != ' '
                    ) return false;
                    if (
                        isSquareAttacked(boardState, 5, 0, 'w') ||
                        isSquareAttacked(boardState, 6, 0, 'w')
                    ) return false;
                    return true;
                }
                // queenside
                if (dx == -2) {
                    if (
                        bRookAMoved ||
                        boardState[0][1] != ' ' ||
                        boardState[0][2] != ' ' ||
                        boardState[0][3] != ' '
                    ) return false;
                    if (
                        isSquareAttacked(boardState, 2, 0, 'w') ||
                        isSquareAttacked(boardState, 3, 0, 'w')
                    ) return false;
                    return true;
                }
            }
        }

        return false;
    }
    }

    return false;
}

// generate all legal moves for a player into the moves array
void generateMoves(char player, Move* moves, int* moveCount) {
    *moveCount = 0;
    char tempBoard[8][8];

    for (int fromY = 0; fromY < 8; ++fromY) {
        for (int fromX = 0; fromX < 8; ++fromX) {
            char piece = board[fromY][fromX];
            if (piece == ' ') continue;
            if ((player == 'w' && !isupper(piece)) || (player == 'b' && !islower(piece))) continue;

            for (int toY = 0; toY < 8; ++toY) {
                for (int toX = 0; toX < 8; ++toX) {
                    if (isValidMove(board, piece, fromX, fromY, toX, toY, player)) {
                        memcpy(tempBoard, board, sizeof(char) * 8 * 8);
                        tempBoard[toY][toX] = piece;
                        tempBoard[fromY][fromX] = ' ';

                        // handle en passant in simulation
                        if (
                            toupper(piece) == 'P' &&
                            toX == enPassantX &&
                            toY == enPassantY
                        ) {
                            if (player == 'w') tempBoard[toY + 1][toX] = ' ';
                            else tempBoard[toY - 1][toX] = ' ';
                        }

                        // handle castling rook in simulation
                        if (
                            toupper(piece) == 'K' &&
                            abs(toX - fromX) == 2
                        ) {
                            if (toX == 6) {
                                tempBoard[fromY][5] = tempBoard[fromY][7];
                                tempBoard[fromY][7] = ' ';
                            } else {
                                tempBoard[fromY][3] = tempBoard[fromY][0];
                                tempBoard[fromY][0] = ' ';
                            }
                        }

                        if (!isKingInCheck(tempBoard, player)) {
                            moves[*moveCount].fromX = fromX;
                            moves[*moveCount].fromY = fromY;
                            moves[*moveCount].toX = toX;
                            moves[*moveCount].toY = toY;
                            (*moveCount)++;
                        }
                    }
                }
            }
        }
    }
}

// convert a move to algebraic notation (piece letter + destination square)
void moveToAlgebraic(char piece, int fromX, int fromY, int toX, int toY, char* output) {
    char pieceName = toupper(piece);
    if (pieceName == 'P') {
        // pawn moves: file + destination for captures, just destination otherwise
        if (fromX != toX) {
            sprintf(output, "%c%d%c%d", 'a' + fromX, 8 - fromY, 'a' + toX, 8 - toY);
        } else {
            sprintf(output, "%c%d", 'a' + toX, 8 - toY);
        }
    } else if (pieceName == 'K' && abs(toX - fromX) == 2) {
        if (toX == 6) strcpy(output, "O-O");
        else strcpy(output, "O-O-O");
    } else {
        sprintf(output, "%c%c%d", pieceName, 'a' + toX, 8 - toY);
    }
}

// append algebraic notation to the game history
void recordMove(char piece, int fromX, int fromY, int toX, int toY) {
    char moveStr[8];
    moveToAlgebraic(piece, fromX, fromY, toX, toY, moveStr);

    size_t len = strlen(gameMovesPlayed);
    if (len > 0 && len < sizeof(gameMovesPlayed) - 1)
        strncat(gameMovesPlayed, " ", sizeof(gameMovesPlayed) - len - 1);
    len = strlen(gameMovesPlayed);
    strncat(gameMovesPlayed, moveStr, sizeof(gameMovesPlayed) - len - 1);

    printf("move: %s  (history: %s)\n", moveStr, gameMovesPlayed);
}

// execute a move on the board (handles en passant, castling, promotion, rights, turn switch)
void executeMove(Move* move) {
    int fromX = move->fromX, fromY = move->fromY;
    int toX = move->toX, toY = move->toY;
    char piece = board[fromY][fromX];

    board[toY][toX] = piece;
    board[fromY][fromX] = ' ';

    // en passant capture
    if (
        toupper(piece) == 'P' &&
        toX == enPassantX &&
        toY == enPassantY
    ) {
        if (currentPlayer == 'w') board[toY + 1][toX] = ' ';
        else board[toY - 1][toX] = ' ';
    }

    // castling rook
    if (
        toupper(piece) == 'K' &&
        abs(toX - fromX) == 2
    ) {
        if (toX == 6) {
            board[fromY][5] = board[fromY][7];
            board[fromY][7] = ' ';
        } else {
            board[fromY][3] = board[fromY][0];
            board[fromY][0] = ' ';
        }
    }

    // promotion (auto-queen for bot)
    if (toupper(piece) == 'P' && (toY == 0 || toY == 7)) {
        board[toY][toX] = (currentPlayer == 'w') ? 'Q' : 'q';
    }

    // reset en passant
    enPassantX = -1;
    enPassantY = -1;

    // set new en passant target
    if (
        toupper(piece) == 'P' &&
        abs(toY - fromY) == 2
    ) {
        enPassantX = toX;
        enPassantY = (currentPlayer == 'w') ? toY + 1 : toY - 1;
    }

    // update castling rights
    if (piece == 'K') wKingMoved = true;
    if (piece == 'k') bKingMoved = true;
    if (piece == 'R' && fromX == 0 && fromY == 7) wRookAMoved = true;
    if (piece == 'R' && fromX == 7 && fromY == 7) wRookHMoved = true;
    if (piece == 'r' && fromX == 0 && fromY == 0) bRookAMoved = true;
    if (piece == 'r' && fromX == 7 && fromY == 0) bRookHMoved = true;

    // record the move in algebraic notation
    recordMove(piece, fromX, fromY, toX, toY);

    // switch turns
    currentPlayer = (currentPlayer == 'w') ? 'b' : 'w';

    checkEndgame();
}

// bot plays: try book first, then alpha-beta search
void makeBotMove() {
    // try opening book first
    Move bookMove;
    if (getBookMove(&bookMove)) {
        executeMove(&bookMove);
        return;
    }

    // generate and order moves
    Move moves[256];
    int moveCount = 0;
    generateMoves(currentPlayer, moves, &moveCount);

    if (moveCount == 0) {
        gameOver = true;
        return;
    }

    orderMoves(moves, moveCount);

    bool botMaximizing = (currentPlayer == 'w');
    int bestScore = botMaximizing ? -INF : INF;
    Move bestMove = moves[0];

    for (int i = 0; i < moveCount; ++i) {
        BoardState saved = saveState();
        searchMakeMove(&moves[i]);
        int score = alphaBeta(SEARCH_DEPTH - 1, -INF, INF);
        restoreState(&saved);

        if (botMaximizing && score > bestScore) {
            bestScore = score;
            bestMove = moves[i];
        } else if (!botMaximizing && score < bestScore) {
            bestScore = score;
            bestMove = moves[i];
        }
    }

    printf("eval: %d\n", bestScore);
    executeMove(&bestMove);
}

bool loadOpeningBook(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) return false;

    // count lines first
    bookSize = 0;
    char line[2048];
    while (fgets(line, sizeof(line), file)) {
        if (line[0] != '\n' && line[0] != '\0')
            bookSize++;
    }

    if (bookSize == 0) { fclose(file); return false; }

    // allocate memory
    openingBook = (BookLine*)malloc(sizeof(BookLine) * bookSize);
    if (!openingBook) { fclose(file); return false; }

    // read the book
    rewind(file);
    int index = 0;
    while (fgets(line, sizeof(line), file) && index < bookSize) {
        if (line[0] == '\n' || line[0] == '\0') continue;

        // find the pipe separator
        char* pipePos = strchr(line, '|');
        if (!pipePos) continue;

        // extract moves (before pipe)
        int moveLen = pipePos - line;
        if (moveLen >= (int)sizeof(openingBook[index].moves))
            moveLen = sizeof(openingBook[index].moves) - 1;
        strncpy(openingBook[index].moves, line, moveLen);
        openingBook[index].moves[moveLen] = '\0';

        // trim trailing spaces from moves
        while (moveLen > 0 && openingBook[index].moves[moveLen - 1] == ' ')
            openingBook[index].moves[--moveLen] = '\0';

        // extract weight (after pipe)
        openingBook[index].weight = atoi(pipePos + 1);

        // extract the next move (last move in the sequence)
        char* lastSpace = strrchr(openingBook[index].moves, ' ');
        if (lastSpace) {
            strncpy(openingBook[index].nextMove, lastSpace + 1,
                   sizeof(openingBook[index].nextMove) - 1);
            openingBook[index].nextMove[sizeof(openingBook[index].nextMove) - 1] = '\0';
        } else {
            strncpy(openingBook[index].nextMove, openingBook[index].moves,
                   sizeof(openingBook[index].nextMove) - 1);
            openingBook[index].nextMove[sizeof(openingBook[index].nextMove) - 1] = '\0';
        }

        index++;
    }

    bookSize = index;
    fclose(file);
    return true;
}

bool algebraicToMove(const char* algebraic, Move* move) {
    if (!algebraic || !move) return false;

    // handle castling
    if (strcmp(algebraic, "O-O") == 0 || strcmp(algebraic, "0-0") == 0) {
        move->fromX = 4;
        move->fromY = (currentPlayer == 'w') ? 7 : 0;
        move->toX = 6;
        move->toY = (currentPlayer == 'w') ? 7 : 0;
        return true;
    }
    if (strcmp(algebraic, "O-O-O") == 0 || strcmp(algebraic, "0-0-0") == 0) {
        move->fromX = 4;
        move->fromY = (currentPlayer == 'w') ? 7 : 0;
        move->toX = 2;
        move->toY = (currentPlayer == 'w') ? 7 : 0;
        return true;
    }

    int len = strlen(algebraic);
    if (len < 2) return false;

    // determine piece type
    char pieceType = 'P';
    int offset = 0;
    if (isupper(algebraic[0])) {
        pieceType = algebraic[0];
        offset = 1;
    }

    // find destination square (last 2 characters, or last 2 before +#)
    int destIdx = len - 1;
    while (destIdx >= 0 && (algebraic[destIdx] == '+' || algebraic[destIdx] == '#'))
        destIdx--;
    if (destIdx < offset + 1) return false;

    int toY = '8' - algebraic[destIdx];
    int toX = algebraic[destIdx - 1] - 'a';
    if (toX < 0 || toX > 7 || toY < 0 || toY > 7) return false;

    // find the piece that can make this move
    char targetPiece = (currentPlayer == 'w') ? pieceType : tolower(pieceType);
    for (int fromY = 0; fromY < 8; fromY++) {
        for (int fromX = 0; fromX < 8; fromX++) {
            if (board[fromY][fromX] == targetPiece) {
                if (isValidMove(board, targetPiece, fromX, fromY, toX, toY, currentPlayer)) {
                    char tempBoard[8][8];
                    memcpy(tempBoard, board, sizeof(board));
                    tempBoard[toY][toX] = targetPiece;
                    tempBoard[fromY][fromX] = ' ';

                    // handle en passant in simulation
                    if (
                        toupper(targetPiece) == 'P' &&
                        toX == enPassantX &&
                        toY == enPassantY
                    ) {
                        if (currentPlayer == 'w') tempBoard[toY + 1][toX] = ' ';
                        else tempBoard[toY - 1][toX] = ' ';
                    }

                    if (!isKingInCheck(tempBoard, currentPlayer)) {
                        move->fromX = fromX;
                        move->fromY = fromY;
                        move->toX = toX;
                        move->toY = toY;
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

bool getBookMove(Move* bookMove) {
    if (!openingBook || bookSize == 0) return false;

    // trim trailing spaces from game moves
    int len = strlen(gameMovesPlayed);
    while (len > 0 && gameMovesPlayed[len - 1] == ' ')
        gameMovesPlayed[--len] = '\0';

    // find all matching lines
    int matchingLines[1000];
    int matchCount = 0;
    int totalWeight = 0;

    for (int i = 0; i < bookSize && matchCount < 1000; i++) {
        if (strcmp(openingBook[i].moves, gameMovesPlayed) == 0) {
            matchingLines[matchCount] = i;
            totalWeight += openingBook[i].weight;
            matchCount++;
        }
    }

    if (matchCount == 0) return false;

    // weighted random selection
    int randomValue = rand() % totalWeight;
    int cumulativeWeight = 0;

    for (int i = 0; i < matchCount; i++) {
        cumulativeWeight += openingBook[matchingLines[i]].weight;
        if (randomValue < cumulativeWeight) {
            if (algebraicToMove(openingBook[matchingLines[i]].nextMove, bookMove)) {
                printf("book move: %s (weight: %d/%d)\n",
                       openingBook[matchingLines[i]].nextMove,
                       openingBook[matchingLines[i]].weight, totalWeight);
                return true;
            }
        }
    }
    return false;
}

// material values
#define PAWN_VAL   100
#define KNIGHT_VAL 320
#define BISHOP_VAL 330
#define ROOK_VAL   500
#define QUEEN_VAL  900
#define KING_VAL   20000

// piece-square tables (from white's perspective, row 0 = rank 8)
static const int pawnTable[8][8] = {
    {  0,  0,  0,  0,  0,  0,  0,  0},
    { 50, 50, 50, 50, 50, 50, 50, 50},
    { 10, 10, 20, 30, 30, 20, 10, 10},
    {  5,  5, 10, 25, 25, 10,  5,  5},
    {  0,  0,  0, 20, 20,  0,  0,  0},
    {  5, -5,-10,  0,  0,-10, -5,  5},
    {  5, 10, 10,-20,-20, 10, 10,  5},
    {  0,  0,  0,  0,  0,  0,  0,  0}
};

static const int knightTable[8][8] = {
    {-50,-40,-30,-30,-30,-30,-40,-50},
    {-40,-20,  0,  0,  0,  0,-20,-40},
    {-30,  0, 10, 15, 15, 10,  0,-30},
    {-30,  5, 15, 20, 20, 15,  5,-30},
    {-30,  0, 15, 20, 20, 15,  0,-30},
    {-30,  5, 10, 15, 15, 10,  5,-30},
    {-40,-20,  0,  5,  5,  0,-20,-40},
    {-50,-40,-30,-30,-30,-30,-40,-50}
};

static const int bishopTable[8][8] = {
    {-20,-10,-10,-10,-10,-10,-10,-20},
    {-10,  0,  0,  0,  0,  0,  0,-10},
    {-10,  0, 10, 10, 10, 10,  0,-10},
    {-10,  5,  5, 10, 10,  5,  5,-10},
    {-10,  0, 10, 10, 10, 10,  0,-10},
    {-10, 10, 10, 10, 10, 10, 10,-10},
    {-10,  5,  0,  0,  0,  0,  5,-10},
    {-20,-10,-10,-10,-10,-10,-10,-20}
};

static const int rookTable[8][8] = {
    {  0,  0,  0,  0,  0,  0,  0,  0},
    {  5, 10, 10, 10, 10, 10, 10,  5},
    { -5,  0,  0,  0,  0,  0,  0, -5},
    { -5,  0,  0,  0,  0,  0,  0, -5},
    { -5,  0,  0,  0,  0,  0,  0, -5},
    { -5,  0,  0,  0,  0,  0,  0, -5},
    { -5,  0,  0,  0,  0,  0,  0, -5},
    {  0,  0,  0,  5,  5,  0,  0,  0}
};

static const int queenTable[8][8] = {
    {-20,-10,-10, -5, -5,-10,-10,-20},
    {-10,  0,  0,  0,  0,  0,  0,-10},
    {-10,  0,  5,  5,  5,  5,  0,-10},
    { -5,  0,  5,  5,  5,  5,  0, -5},
    {  0,  0,  5,  5,  5,  5,  0, -5},
    {-10,  5,  5,  5,  5,  5,  0,-10},
    {-10,  0,  5,  0,  0,  0,  0,-10},
    {-20,-10,-10, -5, -5,-10,-10,-20}
};

// king middlegame table
static const int kingMGTable[8][8] = {
    {-30,-40,-40,-50,-50,-40,-40,-30},
    {-30,-40,-40,-50,-50,-40,-40,-30},
    {-30,-40,-40,-50,-50,-40,-40,-30},
    {-30,-40,-40,-50,-50,-40,-40,-30},
    {-20,-30,-30,-40,-40,-30,-30,-20},
    {-10,-20,-20,-20,-20,-20,-20,-10},
    { 20, 20,  0,  0,  0,  0, 20, 20},
    { 20, 30, 10,  0,  0, 10, 30, 20}
};

// king endgame table (king should be active)
static const int kingEGTable[8][8] = {
    {-50,-40,-30,-20,-20,-30,-40,-50},
    {-30,-20,-10,  0,  0,-10,-20,-30},
    {-30,-10, 20, 30, 30, 20,-10,-30},
    {-30,-10, 30, 40, 40, 30,-10,-30},
    {-30,-10, 30, 40, 40, 30,-10,-30},
    {-30,-10, 20, 30, 30, 20,-10,-30},
    {-30,-30,  0,  0,  0,  0,-30,-30},
    {-50,-30,-30,-30,-30,-30,-30,-50}
};

// count total non-pawn material for a color
int countMaterial(char color) {
    int total = 0;
    for (int r = 0; r < 8; ++r) {
        for (int c = 0; c < 8; ++c) {
            char piece = board[r][c];
            if (piece == ' ') continue;
            bool isWhite = isupper(piece);
            if ((color == 'w' && !isWhite) || (color == 'b' && isWhite)) continue;

            switch (toupper(piece)) {
                case 'N': total += KNIGHT_VAL; break;
                case 'B': total += BISHOP_VAL; break;
                case 'R': total += ROOK_VAL; break;
                case 'Q': total += QUEEN_VAL; break;
            }
        }
    }
    return total;
}

// detect endgame: both sides have low material
int isEndgame() {
    return (countMaterial('w') <= 1300 && countMaterial('b') <= 1300);
}

// count legal moves (mobility) for a color
int getMobility(char color) {
    Move moves[256];
    int moveCount = 0;
    generateMoves(color, moves, &moveCount);
    return moveCount;
}

// evaluate the board from white's perspective
int evaluateBoard() {
    int score = 0;
    int whiteMobility = getMobility('w');
    int blackMobility = getMobility('b');

    for (int r = 0; r < 8; ++r) {
        for (int c = 0; c < 8; ++c) {
            char piece = board[r][c];
            if (piece == ' ') continue;

            bool isWhite = isupper(piece);
            int sign = isWhite ? 1 : -1;

            // material
            int materialValue = 0;
            switch (toupper(piece)) {
                case 'P': materialValue = PAWN_VAL; break;
                case 'N': materialValue = KNIGHT_VAL; break;
                case 'B': materialValue = BISHOP_VAL; break;
                case 'R': materialValue = ROOK_VAL; break;
                case 'Q': materialValue = QUEEN_VAL; break;
                case 'K': materialValue = KING_VAL; break;
            }
            score += sign * materialValue;

            // piece-square table bonus
            // for black, mirror the table vertically (row = 7 - r)
            int pstRow = isWhite ? r : (7 - r);
            int pstCol = c;
            char lookupPiece = isWhite ? piece : toupper(piece);

            // manually lookup the table based on piece type
            int pstBonus = 0;
            switch (toupper(piece)) {
                case 'P': pstBonus = pawnTable[pstRow][pstCol]; break;
                case 'N': pstBonus = knightTable[pstRow][pstCol]; break;
                case 'B': pstBonus = bishopTable[pstRow][pstCol]; break;
                case 'R': pstBonus = rookTable[pstRow][pstCol]; break;
                case 'Q': pstBonus = queenTable[pstRow][pstCol]; break;
                case 'K': {
                    if (isEndgame())
                        pstBonus = kingEGTable[pstRow][pstCol];
                    else
                        pstBonus = kingMGTable[pstRow][pstCol];
                    break;
                }
            }
            score += sign * pstBonus;
        }
    }

    // mobility bonus (3 centipawns per legal move)
    score += (whiteMobility - blackMobility) * 3;

    return score;
}

BoardState saveState() {
    BoardState s;
    memcpy(s.board, board, sizeof(board));
    s.currentPlayer = currentPlayer;
    s.enPassantX = enPassantX;
    s.enPassantY = enPassantY;
    s.wKingMoved = wKingMoved;
    s.bKingMoved = bKingMoved;
    s.wRookAMoved = wRookAMoved;
    s.wRookHMoved = wRookHMoved;
    s.bRookAMoved = bRookAMoved;
    s.bRookHMoved = bRookHMoved;
    return s;
}

void restoreState(BoardState* s) {
    memcpy(board, s->board, sizeof(board));
    currentPlayer = s->currentPlayer;
    enPassantX = s->enPassantX;
    enPassantY = s->enPassantY;
    wKingMoved = s->wKingMoved;
    bKingMoved = s->bKingMoved;
    wRookAMoved = s->wRookAMoved;
    wRookHMoved = s->wRookHMoved;
    bRookAMoved = s->bRookAMoved;
    bRookHMoved = s->bRookHMoved;
}

// execute a move without side effects (no recording, no endgame check)
void searchMakeMove(Move* move) {
    int fromX = move->fromX, fromY = move->fromY;
    int toX = move->toX, toY = move->toY;
    char piece = board[fromY][fromX];

    board[toY][toX] = piece;
    board[fromY][fromX] = ' ';

    // en passant capture
    if (
        toupper(piece) == 'P' &&
        toX == enPassantX &&
        toY == enPassantY
    ) {
        if (currentPlayer == 'w') board[toY + 1][toX] = ' ';
        else board[toY - 1][toX] = ' ';
    }

    // castling rook
    if (
        toupper(piece) == 'K' &&
        abs(toX - fromX) == 2
    ) {
        if (toX == 6) {
            board[fromY][5] = board[fromY][7];
            board[fromY][7] = ' ';
        } else {
            board[fromY][3] = board[fromY][0];
            board[fromY][0] = ' ';
        }
    }

    // promotion (auto-queen)
    if (toupper(piece) == 'P' && (toY == 0 || toY == 7)) {
        board[toY][toX] = (currentPlayer == 'w') ? 'Q' : 'q';
    }

    // reset en passant
    enPassantX = -1;
    enPassantY = -1;

    // set new en passant target
    if (
        toupper(piece) == 'P' &&
        abs(toY - fromY) == 2
    ) {
        enPassantX = toX;
        enPassantY = (currentPlayer == 'w') ? toY + 1 : toY - 1;
    }

    // update castling rights
    if (piece == 'K') wKingMoved = true;
    if (piece == 'k') bKingMoved = true;
    if (piece == 'R' && fromX == 0 && fromY == 7) wRookAMoved = true;
    if (piece == 'R' && fromX == 7 && fromY == 7) wRookHMoved = true;
    if (piece == 'r' && fromX == 0 && fromY == 0) bRookAMoved = true;
    if (piece == 'r' && fromX == 7 && fromY == 0) bRookHMoved = true;

    // switch turns
    currentPlayer = (currentPlayer == 'w') ? 'b' : 'w';
}

// score a move for ordering (MVV-LVA: captures first)
int moveScore(Move* move) {
    char attacker = board[move->fromY][move->fromX];
    char victim = board[move->toY][move->toX];

    // en passant capture
    if (
        toupper(attacker) == 'P' &&
        move->toX == enPassantX &&
        move->toY == enPassantY
    ) {
        return 1000 + 10 - 1; // pawn captures pawn
    }

    if (victim == ' ') return 0;

    int victimVal = 0, attackerVal = 0;
    switch (toupper(victim)) {
        case 'P': victimVal = 1; break;
        case 'N': victimVal = 3; break;
        case 'B': victimVal = 3; break;
        case 'R': victimVal = 5; break;
        case 'Q': victimVal = 9; break;
        case 'K': victimVal = 100; break;
    }
    switch (toupper(attacker)) {
        case 'P': attackerVal = 1; break;
        case 'N': attackerVal = 3; break;
        case 'B': attackerVal = 3; break;
        case 'R': attackerVal = 5; break;
        case 'Q': attackerVal = 9; break;
        case 'K': attackerVal = 100; break;
    }
    return 1000 + victimVal * 10 - attackerVal;
}

// sort moves by score descending (captures first)
void orderMoves(Move* moves, int moveCount) {
    // selection sort (simple, small arrays)
    for (int i = 0; i < moveCount - 1; ++i) {
        int best = i;
        for (int j = i + 1; j < moveCount; ++j) {
            if (moveScore(&moves[j]) > moveScore(&moves[best]))
                best = j;
        }
        if (best != i) {
            Move tmp = moves[i];
            moves[i] = moves[best];
            moves[best] = tmp;
        }
    }
}

// alpha-beta recursive search (returns score from white's perspective)
int alphaBeta(int depth, int alpha, int beta) {
    if (depth == 0) return evaluateBoard();

    bool maximizing = (currentPlayer == 'w');

    Move moves[256];
    int moveCount = 0;
    generateMoves(currentPlayer, moves, &moveCount);

    if (moveCount == 0) {
        if (isKingInCheck(board, currentPlayer)) {
            // checkmate - prefer faster mates with ply adjustment
            int ply = SEARCH_DEPTH - depth;
            return maximizing ? (-INF + ply) : (INF - ply);
        }
        return 0; // stalemate
    }

    orderMoves(moves, moveCount);

    if (maximizing) {
        int best = -INF;
        for (int i = 0; i < moveCount; ++i) {
            BoardState saved = saveState();
            searchMakeMove(&moves[i]);
            int score = alphaBeta(depth - 1, alpha, beta);
            restoreState(&saved);
            if (score > best) best = score;
            if (best > alpha) alpha = best;
            if (beta <= alpha) break;
        }
        return best;
    } else {
        int best = INF;
        for (int i = 0; i < moveCount; ++i) {
            BoardState saved = saveState();
            searchMakeMove(&moves[i]);
            int score = alphaBeta(depth - 1, alpha, beta);
            restoreState(&saved);
            if (score < best) best = score;
            if (best < beta) beta = best;
            if (beta <= alpha) break;
        }
        return best;
    }
}

void renderPromotionChoice() {
    const char* choices = (currentPlayer == 'w') ? "QRNB" : "qrnb";
    int x = promotionX;
    int y = promotionY;

    SDL_SetRenderDrawBlendMode(gRenderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(gRenderer, 100, 100, 100, 180);

    int startY = (y == 0) ? 0 : SCREEN_HEIGHT - 4 * SQUARE_SIZE;
    SDL_Rect bgRect = { x * SQUARE_SIZE, startY, SQUARE_SIZE, 4 * SQUARE_SIZE };
    SDL_RenderFillRect(gRenderer, &bgRect);

    for (int i = 0; i < 4; ++i) {
        char piece = choices[i];
        int drawY = (y == 0) ? i : 7 - i;
        SDL_Rect pieceRect = {
            x * SQUARE_SIZE,
            drawY * SQUARE_SIZE,
            SQUARE_SIZE,
            SQUARE_SIZE
        };
        SDL_RenderCopy(gRenderer, gPieceTextures[(int)piece], NULL, &pieceRect);
    }
}

void handlePromotionClick(int mx, int my) {
    int bx = mx / SQUARE_SIZE;
    int by = my / SQUARE_SIZE;

    if (bx != promotionX) return;

    const char* choices = (currentPlayer == 'w') ? "QRNB" : "qrnb";

    int startY = (promotionY == 0) ? 0 : 4;
    if (by < startY || by >= startY + 4) return;

    int index = (promotionY == 0) ? (by - startY) : (startY + 3 - by);
    char chosenPiece = choices[index];

    board[promotionY][promotionX] = chosenPiece;
    awaitingPromotion = false;
    promotionX = -1;
    promotionY = -1;

    enPassantX = -1;
    enPassantY = -1;

    currentPlayer = (currentPlayer == 'w') ? 'b' : 'w';

    checkEndgame();
}

SDL_Texture* loadTexture(SDL_Renderer* renderer, const char* path) {
    SDL_Surface* surface = IMG_Load(path);

    SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    return tex;
}

bool loadMedia(SDL_Renderer* renderer) {
    const char* names[] = {
        "wP", "wR", "wN", "wB", "wQ", "wK",
        "bP", "bR", "bN", "bB", "bQ", "bK"
    };
    const char ids[] = {
        'P', 'R', 'N', 'B', 'Q', 'K',
        'p', 'r', 'n', 'b', 'q', 'k'
    };

    bool success = true;
    for (int i = 0; i < 12; ++i) {
        char path[256];
        sprintf(path, "img/%s.png", names[i]);
        gPieceTextures[(int)ids[i]] = loadTexture(renderer, path);

        if (!gPieceTextures[(int)ids[i]]) {
            printf("Error: Could not load texture for piece '%c'\n", ids[i]);
            success = false;
        }
    }
    return success;
}

int main(int argc, char* args[]) {
    (void)argc;
    (void)args;
    srand(time(NULL));

    SDL_Window* window = SDL_CreateWindow("chess bot", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    gRenderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    SDL_SetRenderDrawBlendMode(gRenderer, SDL_BLENDMODE_BLEND);

    loadMedia(gRenderer);

    if (loadOpeningBook("komodo.txt")) {
        printf("opening book loaded: %d lines\n", bookSize);
    } else {
        printf("opening book not found, playing random\n");
    }

    bool quit = false;
    SDL_Event e;

    while (!quit) {
        // bot's turn
        if (vsBot && !gameOver && !awaitingPromotion && currentPlayer != humanPlayer) {
            SDL_Delay(500);
            makeBotMove();
        }

        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = true;
            } else if (!gameOver) {
                if (awaitingPromotion) {
                    if (e.type == SDL_MOUSEBUTTONDOWN) {
                        handlePromotionClick(e.button.x, e.button.y);
                    }
                } else if (!vsBot || currentPlayer == humanPlayer) {
                    switch (e.type) {
                        case SDL_MOUSEBUTTONDOWN:
                            if (!isDragging) {
                                int bx = e.button.x / SQUARE_SIZE;
                                int by = e.button.y / SQUARE_SIZE;
                                if (bx >= 0 && bx < 8 && by >= 0 && by < 8 &&
                                    board[by][bx] != ' ' &&
                                    ((currentPlayer == 'w' && isupper(board[by][bx])) ||
                                     (currentPlayer == 'b' && islower(board[by][bx]))))
                                {
                                    isDragging = true;
                                    draggedPiece = board[by][bx];
                                    dragStartX = bx;
                                    dragStartY = by;
                                    mouseX = e.button.x;
                                    mouseY = e.button.y;
                                    board[by][bx] = ' ';
                                }
                            }
                            break;
                        case SDL_MOUSEMOTION:
                            if (isDragging) {
                                mouseX = e.motion.x;
                                mouseY = e.motion.y;
                            }
                            break;
                        case SDL_MOUSEBUTTONUP:
                            if (isDragging) {
                                int bx = e.button.x / SQUARE_SIZE;
                                int by = e.button.y / SQUARE_SIZE;
                                if (bx < 0) bx = 0; if (bx > 7) bx = 7;
                                if (by < 0) by = 0; if (by > 7) by = 7;

                                if (isValidMove(board, draggedPiece, dragStartX, dragStartY, bx, by, currentPlayer)) {
                                    char tempBoard[8][8];
                                    memcpy(tempBoard, board, sizeof(board));
                                    tempBoard[by][bx] = draggedPiece;

                                    if (toupper(draggedPiece) == 'P' && bx == enPassantX && by == enPassantY) {
                                        if (currentPlayer == 'w') tempBoard[by + 1][bx] = ' ';
                                        else tempBoard[by - 1][bx] = ' ';
                                    }
                                    if (toupper(draggedPiece) == 'K' && abs(bx - dragStartX) == 2) {
                                        if (bx == 6) {
                                            tempBoard[dragStartY][5] = tempBoard[dragStartY][7];
                                            tempBoard[dragStartY][7] = ' ';
                                        } else {
                                            tempBoard[dragStartY][3] = tempBoard[dragStartY][0];
                                            tempBoard[dragStartY][0] = ' ';
                                        }
                                    }

                                    if (!isKingInCheck(tempBoard, currentPlayer)) {
                                        memcpy(board, tempBoard, sizeof(board));

                                        if (toupper(draggedPiece) == 'P' && (by == 0 || by == 7)) {
                                            awaitingPromotion = true;
                                            promotionX = bx;
                                            promotionY = by;
                                        } else {
                                            // record the move in algebraic notation
                                            recordMove(draggedPiece, dragStartX, dragStartY, bx, by);

                                            enPassantX = -1; enPassantY = -1;
                                            if (toupper(draggedPiece) == 'P' && abs(by - dragStartY) == 2) {
                                                enPassantX = bx;
                                                enPassantY = (currentPlayer == 'w') ? dragStartY - 1 : dragStartY + 1;
                                            }
                                            if (draggedPiece == 'K') wKingMoved = true;
                                            if (draggedPiece == 'k') bKingMoved = true;
                                            if (draggedPiece == 'R' && dragStartX == 0 && dragStartY == 7) wRookAMoved = true;
                                            if (draggedPiece == 'R' && dragStartX == 7 && dragStartY == 7) wRookHMoved = true;
                                            if (draggedPiece == 'r' && dragStartX == 0 && dragStartY == 0) bRookAMoved = true;
                                            if (draggedPiece == 'r' && dragStartX == 7 && dragStartY == 0) bRookHMoved = true;

                                            currentPlayer = (currentPlayer == 'w') ? 'b' : 'w';
                                            checkEndgame();
                                        }
                                    } else {
                                        board[dragStartY][dragStartX] = draggedPiece;
                                        printf("illegal move - king in check\n");
                                    }
                                } else {
                                    board[dragStartY][dragStartX] = draggedPiece;
                                    printf("illegal move\n");
                                }

                                isDragging = false;
                                draggedPiece = ' ';
                            }
                            break;
                    }
                }
            }
        }

        SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, 255);
        SDL_RenderClear(gRenderer);

        for (int r = 0; r < 8; ++r) {
            for (int c = 0; c < 8; ++c) {
                SDL_Rect square = {
                    c * SQUARE_SIZE, r * SQUARE_SIZE,
                    SQUARE_SIZE, SQUARE_SIZE
                };
                if ((r + c) % 2 == 0)
                    SDL_SetRenderDrawColor(gRenderer, 238, 238, 210, 255);
                else
                    SDL_SetRenderDrawColor(gRenderer, 118, 150, 86, 255);
                SDL_RenderFillRect(gRenderer, &square);
            }
        }

        for (int r = 0; r < 8; ++r) {
            for (int c = 0; c < 8; ++c) {
                char piece = board[r][c];
                if (piece != ' ') {
                    SDL_Rect dest = {
                        c * SQUARE_SIZE, r * SQUARE_SIZE,
                        SQUARE_SIZE, SQUARE_SIZE
                    };
                    SDL_RenderCopy(gRenderer, gPieceTextures[(int)piece], NULL, &dest);
                }
            }
        }

        if (isDragging && draggedPiece != ' ') {
            SDL_Rect dest = {
                mouseX - SQUARE_SIZE / 2,
                mouseY - SQUARE_SIZE / 2,
                SQUARE_SIZE, SQUARE_SIZE
            };
            SDL_RenderCopy(gRenderer, gPieceTextures[(int)draggedPiece], NULL, &dest);
        }

        if (awaitingPromotion)
            renderPromotionChoice();

        SDL_RenderPresent(gRenderer);
    }

    for (int i = 0; i < 128; ++i)
        if (gPieceTextures[i]) SDL_DestroyTexture(gPieceTextures[i]);

    if (openingBook) free(openingBook);
    SDL_DestroyRenderer(gRenderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();
    return 0;
}