#include "chess.h"

bool isSquareAttacked(char boardState[8][8], int x, int y, char attackerColor) {
    for (int r = 0; r < 8; ++r) {
        for (int c = 0; c < 8; ++c) {
            char piece = boardState[r][c];
            if (
                piece != ' ' &&
                ((attackerColor == 'w' && isupper(piece)) ||
                 (attackerColor == 'b' && islower(piece)))
            ) {
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

bool isInsufficientMaterial(void) {
    int wBishops = 0, wKnights = 0, wRooks = 0, wQueens = 0, wPawns = 0;
    int bBishops = 0, bKnights = 0, bRooks = 0, bQueens = 0, bPawns = 0;

    int wBishopSquareColor[10] = {0};
    int bBishopSquareColor[10] = {0};
    int wBishopCount = 0, bBishopCount = 0;

    for (int r = 0; r < 8; ++r) {
        for (int c = 0; c < 8; ++c) {
            char p = board[r][c];
            if (p == ' ') continue;

            int squareColor = (r + c) % 2;

            switch (p) {
                case 'B': wBishops++; wBishopSquareColor[wBishopCount++] = squareColor; break;
                case 'N': wKnights++; break;
                case 'R': wRooks++; break;
                case 'Q': wQueens++; break;
                case 'P': wPawns++; break;
                case 'b': bBishops++; bBishopSquareColor[bBishopCount++] = squareColor; break;
                case 'n': bKnights++; break;
                case 'r': bRooks++; break;
                case 'q': bQueens++; break;
                case 'p': bPawns++; break;
            }
        }
    }

    int wMinor = wBishops + wKnights;
    int bMinor = bBishops + bKnights;

    // if anyone has pawns, rooks, or queens, not insufficient
    if (wPawns > 0 || wRooks > 0 || wQueens > 0) return false;
    if (bPawns > 0 || bRooks > 0 || bQueens > 0) return false;

    // king vs king
    if (wMinor == 0 && bMinor == 0) return true;

    // king + minor piece vs king
    if (wMinor == 1 && bMinor == 0) return true;
    if (wMinor == 0 && bMinor == 1) return true;

    // king + bishop vs king + bishop (both bishops on same color squares)
    if (wBishops == 1 && wKnights == 0 && bBishops == 1 && bKnights == 0) {
        if (wBishopSquareColor[0] == bBishopSquareColor[0])
            return true;
    }

    return false;
}

void checkEndgame(void) {
    if (!hasLegalMoves(currentPlayer)) {
        gameOver = true;
        if (isKingInCheck(board, currentPlayer)) {
            printf("checkmate! %s wins!\n", (currentPlayer == 'w') ? "Black" : "White");
        } else {
            printf("stalemate! draw!\n");
        }
    } else if (isInsufficientMaterial()) {
        gameOver = true;
        printf("insufficient material! draw!\n");
    } else if (isKingInCheck(board, currentPlayer)) {
        printf("%s's turn - check!\n", currentPlayer == 'w' ? "White" : "Black");
    } else {
        printf("%s's turn\n", currentPlayer == 'w' ? "White" : "Black");
    }
}

// ---- move validation ----

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
        int dir = isWhite ? -1 : 1;
        int startRow = isWhite ? 6 : 1;

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
        // normal onesquare move
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