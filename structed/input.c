#include "chess.h"

void handleMouseDown(int x, int y) {
    int bx = x / SQUARE_SIZE;
    int by = y / SQUARE_SIZE;
    if (bx >= 0 && bx < 8 && by >= 0 && by < 8 &&
        board[by][bx] != ' ' &&
        ((currentPlayer == 'w' && isupper(board[by][bx])) ||
         (currentPlayer == 'b' && islower(board[by][bx]))))
    {
        isDragging = true;
        draggedPiece = board[by][bx];
        dragStartX = bx;
        dragStartY = by;
        mouseX = x;
        mouseY = y;
        board[by][bx] = ' ';
    }
}

void handleMouseMotion(int x, int y) {
    if (isDragging) {
        mouseX = x;
        mouseY = y;
    }
}

void handleMouseUp(int x, int y) {
    if (!isDragging) return;

    int bx = x / SQUARE_SIZE;
    int by = y / SQUARE_SIZE;
    if (bx < 0) bx = 0; if (bx > 7) bx = 7;
    if (by < 0) by = 0; if (by > 7) by = 7;

    if (isValidMove(board, draggedPiece, dragStartX, dragStartY, bx, by, currentPlayer)) {
        char tempBoard[8][8];
        memcpy(tempBoard, board, sizeof(board));
        tempBoard[by][bx] = draggedPiece;

        // en passant remove the captured pawn
        if (
            toupper(draggedPiece) == 'P' &&
            bx == enPassantX &&
            by == enPassantY
        ) {
            if (currentPlayer == 'w')
                tempBoard[by + 1][bx] = ' ';
            else
                tempBoard[by - 1][bx] = ' ';
        }

        // castling: move the rook
        if (
            toupper(draggedPiece) == 'K' &&
            abs(bx - dragStartX) == 2
        ) {
            if (bx == 6) {
                // kingside
                tempBoard[dragStartY][5] = tempBoard[dragStartY][7];
                tempBoard[dragStartY][7] = ' ';
            } else {
                // queenside
                tempBoard[dragStartY][3] = tempBoard[dragStartY][0];
                tempBoard[dragStartY][0] = ' ';
            }
        }

        if (!isKingInCheck(tempBoard, currentPlayer)) {
            // legal: commit the move
            memcpy(board, tempBoard, sizeof(board));

            // check for promotion
            if (
                toupper(draggedPiece) == 'P' &&
                (by == 0 || by == 7)
            ) {
                // don't switch turns yet, wait for choice
                awaitingPromotion = true;
                promotionX = bx;
                promotionY = by;
            } else {
                // reset en passant target
                enPassantX = -1;
                enPassantY = -1;

                // set new en passant target if pawn double-pushed
                if (
                    toupper(draggedPiece) == 'P' &&
                    abs(by - dragStartY) == 2
                ) {
                    enPassantX = bx;
                    enPassantY = (currentPlayer == 'w')
                        ? dragStartY - 1
                        : dragStartY + 1;
                }

                // update castling rights
                if (draggedPiece == 'K') wKingMoved = true;
                if (draggedPiece == 'k') bKingMoved = true;
                if (draggedPiece == 'R' && dragStartX == 0 && dragStartY == 7) wRookAMoved = true;
                if (draggedPiece == 'R' && dragStartX == 7 && dragStartY == 7) wRookHMoved = true;
                if (draggedPiece == 'r' && dragStartX == 0 && dragStartY == 0) bRookAMoved = true;
                if (draggedPiece == 'r' && dragStartX == 7 && dragStartY == 0) bRookHMoved = true;

                // switch turns
                currentPlayer = (currentPlayer == 'w') ? 'b' : 'w';

                // check for checkmate/stalemate
                checkEndgame();
            }
        } else {
            // illegal: king would be in check
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

void handlePromotionClick(int mx, int my) {
    int bx = mx / SQUARE_SIZE;
    int by = my / SQUARE_SIZE;

    // must click in the promotion column
    if (bx != promotionX) return;

    const char* choices = (currentPlayer == 'w') ? "QRNB" : "qrnb";

    // determine which of the 4 squares was clicked
    int startY = (promotionY == 0) ? 0 : 4;
    if (by < startY || by >= startY + 4) return;

    int index = (promotionY == 0) ? (by - startY) : (startY + 3 - by);
    char chosenPiece = choices[index];

    // place the chosen piece
    board[promotionY][promotionX] = chosenPiece;
    awaitingPromotion = false;
    promotionX = -1;
    promotionY = -1;

    // reset en passant
    enPassantX = -1;
    enPassantY = -1;

    // switch turns
    currentPlayer = (currentPlayer == 'w') ? 'b' : 'w';

    // check for checkmate/stalemate
    checkEndgame();
}
