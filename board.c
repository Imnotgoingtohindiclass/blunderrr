#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 640
#define SQUARE_SIZE (SCREEN_WIDTH / 8)

/* piece textures indexed by ASCII value of the piece character */
SDL_Texture* gPieceTextures[128] = {NULL};

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

/* ---- move validation ---- */

bool isValidMove(char piece, int fromX, int fromY, int toX, int toY) {
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


    char dest = board[toY][toX];
    /* can't capture own piece */
    if (dest != ' ' &&
        ((isupper(piece) && isupper(dest)) ||
         (islower(piece) && islower(dest))))
        return false;

    int dx = toX - fromX;
    int dy = toY - fromY;

    switch (toupper(piece)) {

    case 'P': {
        bool isWhite = isupper(piece);
        int dir = isWhite ? -1 : 1; /* white moves up, black moves down */
        int startRow = isWhite ? 6 : 1; /* double-push starting row */

        /* move forward one */
        if (dx == 0 && dy == dir && dest == ' ')
            return true;

        /* move forward two from start */
        if (dx == 0 && dy == 2 * dir && fromY == startRow &&
            board[fromY + dir][fromX] == ' ' && dest == ' ')
            return true;

        /* diagonal capture */
        if (abs(dx) == 1 && dy == dir && dest != ' ')
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
            if (board[fromY + i * sy][fromX + i * sx] != ' ')
                return false;
        return true;
    }

    case 'R': {
        if (dx != 0 && dy != 0) return false;
        if (dx == 0) {
            int sy = (dy > 0) ? 1 : -1;
            for (int i = 1; i < abs(dy); ++i)
                if (board[fromY + i * sy][fromX] != ' ')
                    return false;
        } else {
            int sx = (dx > 0) ? 1 : -1;
            for (int i = 1; i < abs(dx); ++i)
                if (board[fromY][fromX + i * sx] != ' ')
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
            if (board[fromY + i * sy][fromX + i * sx] != ' ')
                return false;
        return true;
    }

    case 'K':
        return abs(dx) <= 1 && abs(dy) <= 1;
    }

    return false;
}

/* ---- helpers ---- */

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
    for (int i = 0; i < 12; ++i) {
        char path[256];
        sprintf(path, "img/%s.png", names[i]);
        gPieceTextures[(int)ids[i]] = loadTexture(renderer, path);
        if (!gPieceTextures[(int)ids[i]]) return false;
    }
    return true;
}

int main(int argc, char* args[]) {
    (void)argc;
    (void)args;
    int imgFlags = IMG_INIT_PNG;

    SDL_Window* window = SDL_CreateWindow("chess board", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    /* drag state */
    bool isDragging = false;
    char draggedPiece = ' ';
    int dragStartX = -1, dragStartY = -1;
    int mouseX = 0, mouseY = 0;

    bool quit = false;
    SDL_Event e;

    while (!quit) {
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = true;

            } else if (e.type == SDL_MOUSEBUTTONDOWN && !isDragging) {
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

            } else if (e.type == SDL_MOUSEMOTION && isDragging) {
                mouseX = e.motion.x;
                mouseY = e.motion.y;

            } else if (e.type == SDL_MOUSEBUTTONUP && isDragging) {
                int bx = e.button.x / SQUARE_SIZE;
                int by = e.button.y / SQUARE_SIZE;
                if (bx < 0) bx = 0; if (bx > 7) bx = 7;
                if (by < 0) by = 0; if (by > 7) by = 7;

                if (isValidMove(draggedPiece, dragStartX, dragStartY, bx, by)) {
                    board[by][bx] = draggedPiece;
                    currentPlayer = (currentPlayer == 'w') ? 'b' : 'w';
                    printf("%s's turn\n", currentPlayer == 'w' ? "White" : "Black");
                } else {
                    board[dragStartY][dragStartX] = draggedPiece; /* snap back */
                    printf("illegal move\n");
                }

                isDragging = false;
                draggedPiece = ' ';
            }
        }

        /* draw the 8x8 board */
        for (int r = 0; r < 8; ++r) {
            for (int c = 0; c < 8; ++c) {
                SDL_Rect square = {
                    c * SQUARE_SIZE, r * SQUARE_SIZE,
                    SQUARE_SIZE, SQUARE_SIZE
                };
                if ((r + c) % 2 == 0)
                    SDL_SetRenderDrawColor(renderer, 238, 238, 210, 255);
                else
                    SDL_SetRenderDrawColor(renderer, 118, 150, 86, 255);
                SDL_RenderFillRect(renderer, &square);
            }
        }

        /* draw pieces on the board */
        for (int r = 0; r < 8; ++r) {
            for (int c = 0; c < 8; ++c) {
                char piece = board[r][c];
                if (piece != ' ') {
                    SDL_Rect dest = {
                        c * SQUARE_SIZE, r * SQUARE_SIZE,
                        SQUARE_SIZE, SQUARE_SIZE
                    };
                    SDL_RenderCopy(renderer, gPieceTextures[(int)piece], NULL, &dest);
                }
            }
        }

        /* draw the dragged piece on top */
        if (isDragging && draggedPiece != ' ') {
            SDL_Rect dest = {
                mouseX - SQUARE_SIZE / 2,
                mouseY - SQUARE_SIZE / 2,
                SQUARE_SIZE, SQUARE_SIZE
            };
            SDL_RenderCopy(renderer, gPieceTextures[(int)draggedPiece], NULL, &dest);
        }

        SDL_RenderPresent(renderer);
    }

    for (int i = 0; i < 128; ++i)
        if (gPieceTextures[i]) SDL_DestroyTexture(gPieceTextures[i]);

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();
    return 0;
}