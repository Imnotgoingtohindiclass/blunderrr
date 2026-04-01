#ifndef CHESS_H
#define CHESS_H

#include <SDL.h>
#include <SDL_image.h>
#include <stdbool.h>
#include <ctype.h>

#define SCREEN_WIDTH  640
#define SCREEN_HEIGHT 640
#define SQUARE_SIZE (SCREEN_WIDTH / 8)

extern SDL_Window* gWindow;
extern SDL_Renderer* gRenderer;
extern SDL_Texture* gPieceTextures[128];

extern char board[8][8];
extern char currentPlayer;
extern bool gameOver;
extern bool stalemate;

extern bool isDragging;
extern char draggedPiece;
extern SDL_Point dragStartPosition;
extern SDL_Point mousePosition;

extern bool awaitingPromotion;
extern SDL_Point promotionSquare;

extern bool wKingMoved, bKingMoved;
extern bool wRookAMoved, wRookHMoved;
extern bool bRookAMoved, bRookHMoved;

extern int mouseX;
extern int mouseY;
extern int promotionX;
extern int promotionY;

extern int dragStartX;
extern int dragStartY;
extern int enPassantX;
extern int enPassantY;

extern SDL_Point enPassantTargetSquare;

bool init(void);
bool loadMedia(SDL_Renderer* renderer);
void close_app(void);
SDL_Texture* loadTexture(SDL_Renderer* renderer, const char* path);

void renderBoard(void);
void renderPieces(void);
void renderPromotionChoice(void);

void handleMouseDown(int x, int y);
void handleMouseMotion(int x, int y);
void handleMouseUp(int x, int y);
void handlePromotionClick(int x, int y);
void checkEndgame(void);

bool isValidMove(char boardState[8][8], char piece, int fromX, int fromY, int toX, int toY, char player);
bool isSquareAttacked(char boardState[8][8], int x,int y, char attackerColor);
bool isKingInCheck(char boardState[8][8], char kingColor);
bool hasLegalMoves(char playerColor);

#endif /* CHESS_H */
