#include "chess.h"

// globs
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

bool wKingMoved = false, bKingMoved = false;
bool wRookAMoved = false, wRookHMoved = false;
bool bRookAMoved = false, bRookHMoved = false;

bool gameOver = false;

bool awaitingPromotion = false;
int  promotionX = -1;
int  promotionY = -1;

bool isDragging = false;
char draggedPiece = ' ';
int  mouseX = 0, mouseY = 0;

int dragStartX = -1;
int dragStartY = -1;
int enPassantX = -1;
int enPassantY = -1;

SDL_Texture* loadTexture(SDL_Renderer* renderer, const char* path) {
    SDL_Surface* surface = IMG_Load(path);
    if (!surface) {
        printf("Failed to load image %s! SDL_image Error: %s\n", path, IMG_GetError());
        return NULL;
    }

    SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surface);
    if (!tex) {
        printf("Failed to create texture from %s! SDL Error: %s\n", path, SDL_GetError());
    }

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

void cleanup(SDL_Window* window) {
    for (int i = 0; i < 128; ++i)
        if (gPieceTextures[i]) SDL_DestroyTexture(gPieceTextures[i]);

    SDL_DestroyRenderer(gRenderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();
}

// main
int main(int argc, char* args[]) {
    (void)argc;
    (void)args;
    // int imgFlags = IMG_INIT_PNG;

    SDL_Window* window = SDL_CreateWindow("chess board", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    gRenderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    SDL_SetRenderDrawBlendMode(gRenderer, SDL_BLENDMODE_BLEND);

    loadMedia(gRenderer);

    bool quit = false;
    SDL_Event e;

    while (!quit) {
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = true;

            } else if (gameOver) {
                // game is over, ignore input

            } else if (awaitingPromotion) {
                // only handle promotion clicks while waiting
                if (e.type == SDL_MOUSEBUTTONDOWN)
                    handlePromotionClick(e.button.x, e.button.y);

            } else {
                switch (e.type) {
                    case SDL_MOUSEBUTTONDOWN:
                        handleMouseDown(e.button.x, e.button.y);
                        break;
                    case SDL_MOUSEMOTION:
                        handleMouseMotion(e.motion.x, e.motion.y);
                        break;
                    case SDL_MOUSEBUTTONUP:
                        handleMouseUp(e.button.x, e.button.y);
                        break;
                }
            }
        }

        // clear screen
        SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, 255);
        SDL_RenderClear(gRenderer);
        // draw board
        renderBoard();
        // draw pieces
        renderPieces();
        // draw promotion
        if (awaitingPromotion)
            renderPromotionChoice();

        SDL_RenderPresent(gRenderer);
    }

    cleanup(window);
    return 0;
}