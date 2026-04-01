#include "chess.h"

void renderBoard(void) {
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
}

void renderPieces(void) {
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
}

void renderPromotionChoice(void) {
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
