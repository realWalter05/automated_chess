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
extern bool playingAsWhite;
extern int kingsCordinates[4];
extern short Q;
extern short O;
extern short K;
extern short R;
extern short k;

