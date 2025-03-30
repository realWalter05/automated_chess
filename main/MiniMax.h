// Minimax algorithm params
unsigned short myrand(void);
void bkp();
void serialBoard();
void getAIMove(char move[4]);
void getKingsCordinates();
extern char lastMove[], lastMoveAI[];
extern bool validMove;
extern int gameStatus;
extern int difficulty;
extern int kingsCordinates[4];

