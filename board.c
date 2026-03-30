#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#define SCREEN_WIDTH  640
#define SCREEN_HEIGHT 640
#define SQUARE_SIZE   (SCREEN_WIDTH / 8)

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

    SDL_Window* window = SDL_CreateWindow("chess board", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,  SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    bool quit = false;
    SDL_Event e;

    while (!quit) {
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT)
                quit = true;
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

        /* draw pieces */
        for (int r = 0; r < 8; ++r) {
            for (int c = 0; c < 8; ++c) {
                char piece = board[r][c];
                if (piece != ' ') {
                    SDL_Rect dest = {
                        c * SQUARE_SIZE, r * SQUARE_SIZE,
                        SQUARE_SIZE, SQUARE_SIZE
                    };
                    SDL_RenderCopy(renderer, gPieceTextures[(int)piece],
                                   NULL, &dest);
                }
            }
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