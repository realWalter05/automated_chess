#include "MiniMax.h"

// General
int boxLength = 135;
int motorDelay = 1200;
bool gameStarted = false;

int boardValues[8][8];
int boardValuesMemory[8][8];
int recordedReedValue[8];
int controlValues[8];

// Detect values
char move[4] = {0, 0, 0, 0};
char numberTranslate[8] = {'1', '2', '3', '4', '5', '6', '7', '8'};
char letterTranslate[8] = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h'};
char aiLetterTranslate[8] = "abcdefgh";

int kingsCordinates[4] = {0, 0, 0, 0};

// Magnet
const int magnetPin = 13;
bool magnetActivated = LOW;
bool magnetReady = false;

int magnetX = 0;
int magnetY = 0;

// Motors
const int stepPinX = 12;
const int dirPinX = 11;

const int stepPinY = 10;
const int dirPinY = 9;

// MUX
int muxEnable[5] = {A0, 4, 5, 6, 7};
int muxAddr[4] = {A1, A2, A3, A4};
int muxOutput = A5;

// Control MUX
bool playingAsWhite = true;
bool userTurn = true;
int difficulty = 0;
short int aiPlayColor = 16;

/* ------------------ */
/* ARDUINO SETUP FUNC */
/* ------------------ */
void setup() {
  // DEBUG info
  Serial.begin(9600);

  // Electromagnet
  pinMode(magnetPin, OUTPUT);
  digitalWrite(magnetPin, LOW);

  //// MOTORS ////
  pinMode(stepPinX, OUTPUT);
  pinMode(dirPinX, OUTPUT);
  pinMode(stepPinY, OUTPUT);
  pinMode(dirPinY, OUTPUT);

  //// MULTIPLEXERS ////

  // Setting multiplexer output as Input_pullup
  pinMode(muxOutput, INPUT_PULLUP);

  // Setting default board and control values
  for (int i = 0; i < 8; i++) {
    for (int j = 0; j < 8; j++) {
      boardValues[i][j] = 1;
    }
  }

  for (int i = 0; i < 5; i++) {
    controlValues[i] = 1;
  }

  // Disabling Multiplexers  (When high - does nothing)
  for (int i = 0; i < 5; i++) {
    pinMode(muxEnable[i], OUTPUT);
    digitalWrite(muxEnable[i], HIGH);
  }

  for (int i = 0; i < 4; i++) {
    pinMode(muxAddr[i], OUTPUT);
    digitalWrite(muxAddr[i], LOW);
  }
}

/* ------------------ */
/* ARDUINO MAIN FUNC */
/* ------------------ */
void loop() {
  if (!gameStarted || (magnetX == 0 && magnetY == 0)) {
    // Waiting for user to put pieces in the control part of the board
    if (magnetX == 0 && magnetY == 0) {
      delay(2000);
      resetMagnet();
      return;
    }

    // Check control multiplexor
    delay(5000);
    setControlMUX();
    return;
  }

  if (playingAsWhite == false && lastMoveAI[0] == lastMoveAI[1]) {
    // AI starting as BLACK
    setCurrentBoard();
    memcpy(boardValuesMemory, boardValues, sizeof(boardValuesMemory));

    Serial.println("AI starts");
    getAIMove("");
    makeChessMove(lastMoveAI);
    resetMove();
  }

  // User's turn
  setCurrentBoard();
  memcpy(boardValuesMemory, boardValues, sizeof(boardValuesMemory));
  while (!move[0] || !move[1] || !move[2] || !move[3]) {
    detectBoardMovement();
  }

  // Debug info
  Serial.print("User: ");
  Serial.print(move[0]);
  Serial.print(move[1]);
  Serial.print(" -> ");
  Serial.print(move[2]);
  Serial.println(move[3]);

  // AI turn
  Serial.println("Ai playing...");
  getAIMove(move);

  // Checking game
  if (!validMove ||
      ((lastMoveAI[0] == lastMoveAI[2]) && (lastMoveAI[1] == lastMoveAI[3]))) {
    // If user made invalid move
    resetMove();
    return;
  }

  if (gameStatus == 1 || gameStatus == 2) {
    getKingsCordinates();

    Serial.print("Kings cordinates: ");
    Serial.print(kingsCordinates[0]);
    Serial.print(kingsCordinates[1]);
    Serial.print("/");
    Serial.print(kingsCordinates[2]);
    Serial.println(kingsCordinates[3]);

    // Moving kings out of chess board
    handlePieceTaking(magnetX, magnetY, kingsCordinates[0], kingsCordinates[1]);
    handlePieceTaking(magnetX, magnetY, kingsCordinates[2], kingsCordinates[3]);

    // Reset game
    magnetX = 0;
    magnetY = 0;
    gameStarted = false;
  }

  // Make AI move
  Serial.println(lastMoveAI);
  makeChessMove(lastMoveAI);

  resetMove();
}

/* ----------- */
/* HELPER FUNC */
/* ----------- */
bool isPlaceOccupied(int x, int y) {
  if (!boardValues[y - 1][x - 1]) {
    return true;
  }
  return false;
}

void resetMove() {
  move[0] = 0;
  move[1] = 0;
  move[2] = 0;
  move[3] = 0;
}

/* ----------------- */
/* DETECT BOARD FUNC */
/* ----------------- */
void getReedValues(int targetMuxAddress, int from, int to) {
  // Splitting to two rows because row of chessboard has 8 pieces
  int muxValues[8];

  // Activating MUX
  digitalWrite(targetMuxAddress, LOW);

  for (int j = from; j < to; j++) {
    // Checking MUX combinations
    digitalWrite(muxAddr[0], j % 2);
    digitalWrite(muxAddr[1], j / 2 % 2);
    digitalWrite(muxAddr[2], j / 4 % 2);
    digitalWrite(muxAddr[3], j / 8 % 2);

    int reedValue = digitalRead(muxOutput);

    if (j > 7) {
      muxValues[j - 8] = reedValue;
    } else {
      muxValues[to - j - 1] = reedValue;
    }
  }

  // Resetting MUX
  digitalWrite(muxAddr[0], LOW);
  digitalWrite(muxAddr[1], LOW);
  digitalWrite(muxAddr[2], LOW);
  digitalWrite(muxAddr[3], LOW);

  digitalWrite(targetMuxAddress, HIGH);
  memcpy(recordedReedValue, muxValues, sizeof(recordedReedValue));
}

void setCurrentBoard() {
  getReedValues(muxEnable[0], 0, 8);
  memcpy(boardValues[0], recordedReedValue, sizeof(boardValues[0]));

  getReedValues(muxEnable[0], 8, 16);
  memcpy(boardValues[1], recordedReedValue, sizeof(boardValues[1]));

  getReedValues(muxEnable[1], 0, 8);
  memcpy(boardValues[2], recordedReedValue, sizeof(boardValues[2]));

  getReedValues(muxEnable[1], 8, 16);
  memcpy(boardValues[3], recordedReedValue, sizeof(boardValues[3]));

  getReedValues(muxEnable[2], 0, 8);
  memcpy(boardValues[4], recordedReedValue, sizeof(boardValues[4]));

  getReedValues(muxEnable[2], 8, 16);
  memcpy(boardValues[5], recordedReedValue, sizeof(boardValues[5]));

  getReedValues(muxEnable[3], 0, 8);
  memcpy(boardValues[6], recordedReedValue, sizeof(boardValues[6]));

  getReedValues(muxEnable[3], 8, 16);
  memcpy(boardValues[7], recordedReedValue, sizeof(boardValues[7]));
}

void detectBoardMovement() {
  bool fromChange = false;
  setCurrentBoard();

  Serial.println("////// Board //////");
  for (int first = 7; first >= 0; first--) {
    Serial.print(first + 1);
    Serial.print(" / ");
    for (int second = 0; second < 8; second++) {
      Serial.print(boardValues[first][second]);
      Serial.print(" ");
    }
    Serial.println();
  }
  Serial.println("    A B C D E F G H");

  Serial.print("Moves:");
  Serial.print(move[0]);
  Serial.print(move[1]);
  Serial.print("/");
  Serial.print(move[2]);
  Serial.println(move[3]);

  for (int i = 0; i < 8; i++) {
    for (int j = 0; j < 8; j++) {
      if (boardValuesMemory[i][j] != boardValues[i][j] && boardValues[i][j]) {
        // Position of piece has changed
        delay(1000);
        setCurrentBoard();
        if (boardValues[i][j]) {
          // Piece was here before
          fromChange = true;
          move[0] = letterTranslate[j];
          move[1] = numberTranslate[i];
        }
      }
    }
  }

  for (int i1 = 0; i1 < 8; i1++) {
    for (int j1 = 0; j1 < 8; j1++) {
      if (boardValuesMemory[i1][j1] != boardValues[i1][j1] &&
          !boardValues[i1][j1]) {
        delay(1000);
        setCurrentBoard();
        if (!boardValues[i1][j1]) {
          // New piece on this position
          move[2] = letterTranslate[j1];
          move[3] = numberTranslate[i1];
        }
      }
    }
  }

  if (!fromChange) {
    setCurrentBoard();
    memcpy(boardValuesMemory, boardValues, sizeof(boardValuesMemory));
    resetMove();
  }
  delay(1000);
}

/* ---------- */
/* SETUP FUNC */
/* ---------- */
void resetMagnet() {
  getReedValues(muxEnable[4], 0, 8);
  memcpy(controlValues, recordedReedValue, sizeof(controlValues));

  Serial.println("Magnet reset values");
  Serial.print(controlValues[2]);
  Serial.print(controlValues[3]);
  Serial.print(controlValues[4]);
  Serial.print(controlValues[5]);
  Serial.println(controlValues[6]);

  while (!controlValues[4] && !controlValues[5] && !controlValues[6]) {
    getReedValues(muxEnable[4], 0, 8);
    memcpy(controlValues, recordedReedValue, sizeof(controlValues));
    digitalWrite(dirPinX, LOW);
    digitalWrite(stepPinX, HIGH);
    delayMicroseconds(motorDelay + 600);
    digitalWrite(stepPinX, LOW);
    delayMicroseconds(motorDelay + 600);
    magnetX = 1;
  }

  while (controlValues[4] && !controlValues[5] && !controlValues[6]) {
    getReedValues(muxEnable[4], 0, 8);
    memcpy(controlValues, recordedReedValue, sizeof(controlValues));

    digitalWrite(dirPinY, HIGH);
    digitalWrite(stepPinY, HIGH);
    delayMicroseconds(motorDelay + 600);
    digitalWrite(stepPinY, LOW);
    delayMicroseconds(motorDelay + 600);
    magnetY = 1;
  }
  digitalWrite(dirPinY, LOW);
}

void setControlMUX() {
  getReedValues(muxEnable[4], 0, 8);
  memcpy(controlValues, recordedReedValue, sizeof(controlValues));

  // Setting the data based on MUX wiring
  if (!controlValues[6]) {
    playingAsWhite = true;
  }

  if (!controlValues[5] && controlValues[6]) {
    playingAsWhite = false;
  }

  if (!controlValues[4]) {
    difficulty = 0;
  }

  if (!controlValues[3]) {
    difficulty = 1;
  }

  if (!controlValues[2]) {
    difficulty = 2;
  }

  Serial.print("Control MUX: ");
  Serial.print(controlValues[6]);
  Serial.print(controlValues[5]);
  Serial.print(controlValues[4]);
  Serial.print(controlValues[3]);
  Serial.println(controlValues[2]);
  if ((!controlValues[6] || !controlValues[5]) &&
      (!controlValues[4] || !controlValues[3] || !controlValues[2])) {
    // Checking if mandatory user info is set

    Serial.println("Game started");
    setupMagnet();
    gameStarted = true;
  }
}

void setupMagnet() {
  delay(2000);
  digitalWrite(dirPinX, HIGH);
  for (int j = 0; j < 210; j++) {
    digitalWrite(stepPinX, HIGH);
    delayMicroseconds(motorDelay + 600);
    digitalWrite(stepPinX, LOW);
    delayMicroseconds(motorDelay + 600);
  }
  digitalWrite(dirPinX, LOW);

  digitalWrite(dirPinY, HIGH);
  for (int i = 0; i < 40; i++) {
    digitalWrite(stepPinY, HIGH);
    delayMicroseconds(motorDelay + 600);
    digitalWrite(stepPinY, LOW);
    delayMicroseconds(motorDelay + 600);
  }
  digitalWrite(dirPinY, LOW);
}

/* ------------- */
/* MOVEMENT FUNC */
/* ------------- */

void moveMagnet(int direction, int distance) {
  distance = abs(distance);

  switch (direction) {
  // Direction of the movement based on position of key in numeric keypad
  case 1:
    // Diagonal left down
    digitalWrite(dirPinY, HIGH);
    digitalWrite(dirPinX, LOW);
    for (int i = 0; i < boxLength * distance; i++) {
      digitalWrite(stepPinY, HIGH);
      digitalWrite(stepPinX, HIGH);
      delayMicroseconds(motorDelay);
      digitalWrite(stepPinY, LOW);
      digitalWrite(stepPinX, LOW);
      delayMicroseconds(motorDelay);
    }
    magnetX = magnetX - distance;
    magnetY = magnetY - distance;
    break;

  case 2:
    // Down
    digitalWrite(dirPinY, HIGH);
    for (int i = 0; i < boxLength * distance; i++) {
      digitalWrite(stepPinY, HIGH);
      delayMicroseconds(motorDelay);
      digitalWrite(stepPinY, LOW);
      delayMicroseconds(motorDelay);
    }
    magnetY = magnetY - distance;
    break;

  case 3:
    // Diagonal right down
    digitalWrite(dirPinY, HIGH);
    digitalWrite(dirPinX, HIGH);
    for (int i = 0; i < boxLength * 1.08 * distance; i++) {
      digitalWrite(stepPinY, HIGH);
      digitalWrite(stepPinX, HIGH);
      delayMicroseconds(motorDelay);
      digitalWrite(stepPinY, LOW);
      digitalWrite(stepPinX, LOW);
      delayMicroseconds(motorDelay);
    }
    magnetX = magnetX + distance;
    magnetY = magnetY - distance;
    break;

  case 4:
    // Left
    digitalWrite(dirPinX, LOW);
    for (int i = 0; i < boxLength * distance; i++) {
      digitalWrite(stepPinX, HIGH);
      delayMicroseconds(motorDelay);
      digitalWrite(stepPinX, LOW);
      delayMicroseconds(motorDelay);
    }
    magnetX = magnetX - distance;
    break;

  case 6:
    // Right
    digitalWrite(dirPinX, HIGH);
    for (int i = 0; i < boxLength * distance; i++) {
      digitalWrite(stepPinX, HIGH);
      delayMicroseconds(motorDelay);
      digitalWrite(stepPinX, LOW);
      delayMicroseconds(motorDelay);
    }
    magnetX = magnetX + distance;
    break;

  case 7:
    // Diagonal left up
    digitalWrite(dirPinY, LOW);
    digitalWrite(dirPinX, LOW);
    for (int i = 0; i < boxLength * 1.08 * distance; i++) {
      digitalWrite(stepPinY, HIGH);
      digitalWrite(stepPinX, HIGH);
      delayMicroseconds(motorDelay);
      digitalWrite(stepPinY, LOW);
      digitalWrite(stepPinX, LOW);
      delayMicroseconds(motorDelay);
    }
    magnetX = magnetX - distance;
    magnetY = magnetY + distance;
    break;

  case 8:
    // Up
    digitalWrite(dirPinY, LOW);
    for (int i = 0; i < boxLength * distance; i++) {
      digitalWrite(stepPinY, HIGH);
      delayMicroseconds(motorDelay);
      digitalWrite(stepPinY, LOW);
      delayMicroseconds(motorDelay);
    }
    magnetY = magnetY + distance;
    break;

  case 9:
    // Diagonal right up
    digitalWrite(dirPinY, LOW);
    digitalWrite(dirPinX, HIGH);
    for (int i = 0; i < boxLength * distance; i++) {
      digitalWrite(stepPinY, HIGH);
      digitalWrite(stepPinX, HIGH);
      delayMicroseconds(motorDelay);
      digitalWrite(stepPinY, LOW);
      digitalWrite(stepPinX, LOW);
      delayMicroseconds(motorDelay);
    }
    magnetX = magnetX + distance;
    magnetY = magnetY + distance;
    break;
  }

  // Setting to default
  digitalWrite(dirPinX, LOW);
  digitalWrite(dirPinY, LOW);
  digitalWrite(stepPinX, LOW);
  digitalWrite(stepPinY, LOW);
}

void makeMove(int x, int y, int targetX, int targetY, bool magnetActivated) {
  if (magnetActivated) {
    digitalWrite(magnetPin, HIGH);
    delay(500);
  }

  if (abs(y - targetY) == abs(targetX - x)) {
    // Diagonal movement when moving piece
    if (y > targetY) {
      // Diagonal down
      if (x < targetX) {
        // Right
        moveMagnet(3, targetX - x);
      } else {
        // Left
        moveMagnet(1, targetX - x);
      }
    } else {
      if (x < targetX) {
        // Right
        moveMagnet(9, targetX - x);
      } else {
        // Left
        moveMagnet(7, targetX - x);
      }
    }
    Serial.print(targetX);
    Serial.println(targetY);
    if (magnetActivated) {
      digitalWrite(magnetPin, LOW);
    }
    return;
  }

  // Move magnet to starting position on Y
  if (y < targetY) {
    // Move up
    moveMagnet(8, y - targetY);
  } else if (y > targetY) {
    // Move down
    moveMagnet(2, targetY - y);
  }

  // Move magnet to starting position on X
  if (x < targetX) {
    // Move to right
    moveMagnet(6, x - targetX);
  } else if (x > targetX) {
    // Move to left
    moveMagnet(4, targetX - x);
  }

  if (magnetActivated) {
    digitalWrite(magnetPin, LOW);
  }
}

void makeChessMove(char givenMove[5]) {
  // Deconstruct move
  Serial.println("making chess move");
  Serial.println(givenMove);
  int fromX = strchr(aiLetterTranslate, givenMove[0]) - aiLetterTranslate + 1;
  int fromY = givenMove[1] - '0';

  int toX = strchr(aiLetterTranslate, givenMove[2]) - aiLetterTranslate + 1;
  int toY = givenMove[3] - '0';

  // Taking piece
  if (isPlaceOccupied(toX, toY)) {
    Serial.println("Taking a piece");
    handlePieceTaking(fromX, fromY, toX, toY);
  }

  // Prepare magnet
  makeMove(magnetX, magnetY, fromX, fromY, false);

  // Piece Movement options
  if (fromX == 5 && (toX == 7 || toX == 3) &&
      (toY == 1 && fromY == 1 || fromY == 8 && toY == 8)) {
    // Castling
    Serial.println("Castling");
    handleCastling(fromX, fromY, toX, toY);
  } else if (abs(fromY - toY) != abs(toX - fromX) &&
             (abs(fromX - toX) == 1 || abs(fromX - toX) == 2)) {
    // Horse movement
    Serial.println("Horse movement");
    handleHorseMovement(fromX, fromY, toX, toY);
  } else {
    // Other pieces
    makeMove(fromX, fromY, toX, toY, true);
  }
  delay(1000);
}

void handlePieceTaking(int fromX, int fromY, int toX, int toY) {
  // Prepare magnet to taken piece
  makeMove(magnetX, magnetY, toX, toY, false);

  int movingInRow = toY;
  for (int blockRightX = toX; blockRightX < 9; blockRightX++) {
    // Move taken piece out of chess board
    if (isPlaceOccupied(blockRightX + 1, movingInRow)) {
      if (movingInRow != 0) {
        if (!isPlaceOccupied(blockRightX, movingInRow - 1)) {
          // If piece bellow empty, move there and then continue right
          makeMove(blockRightX, movingInRow, blockRightX + 1, movingInRow - 1,
                   true);
          movingInRow = movingInRow - 1;
          continue;
        }
      }

      if (movingInRow != 8) {
        if (!isPlaceOccupied(blockRightX, movingInRow + 1)) {
          // If piece above empty, move there and then continue right
          makeMove(blockRightX, movingInRow, blockRightX + 1, movingInRow + 1,
                   true);
          movingInRow = movingInRow + 1;
          continue;
        }
      }
    }

    // If empty, move it right
    makeMove(blockRightX, movingInRow, blockRightX + 1, movingInRow, true);
  }
}

void handleCastling(int fromX, int fromY, int toX, int toY) {
  int awayY = 7;
  int awayPawnY = 6;
  if (toY == 1) {
    awayY = 2;
    awayPawnY = 3;
  }

  if (fromX < toX) {
    // Castle to right
    // Move rook away
    makeMove(magnetX, magnetY, toX, toY, false);
    makeMove(toX + 1, toY, 9, awayY, true);

    // Moving king away
    makeMove(9, awayY, fromX, fromY, false);
    makeMove(fromX, fromY, 9, toY, true);

    // Putting rook in place
    makeMove(9, toY, 9, awayY, false);
    makeMove(9, awayY, 8, toY, true);
    makeMove(8, toY, toX - 1, toY, true);

    // Putting king in place
    makeMove(toX - 1, toY, 9, toY, false);
    makeMove(9, toY, toX, toY, true);
  } else {
    // Castle to left
    if (isPlaceOccupied(4, awayY)) {
      // Puting pawn away
      makeMove(magnetX, magnetY, 4, awayY, false);
      makeMove(4, awayY, 4, awayPawnY, true);
      makeMove(4, awayPawnY, fromX, fromY, false);
    }
    // Moving king away
    makeMove(fromX, fromY, 4, awayY, true);

    // Putting rook in place
    makeMove(4, awayY, 1, fromY, false);
    makeMove(1, fromY, 4, fromY, true);

    // Putting king in place
    makeMove(4, fromY, 4, awayY, false);
    makeMove(4, awayY, 3, fromY, true);
  }
}

void handleHorseMovement(int fromX, int fromY, int toX, int toY) {
  // Detected by not diagonal and not in same line or row
  if (abs(fromX - toX) == 1) {
    Serial.println("horse movement vertical");
    // Vertical horse movement
    int awayY = fromY - 1;
    int awayPawnY = fromY - 3;
    if (fromY < toY) {
      awayY = fromY + 1;
      awayPawnY = fromY + 3;
    }

    if (!isPlaceOccupied(fromX, awayY)) {
      // Empty in front him
      makeMove(fromX, fromY, fromX, awayY, true);
      makeMove(fromX, awayY, toX, toY, true);
      return;
    }

    if (fromX != 0 && !isPlaceOccupied(fromX - 1, awayY)) {
      makeMove(fromX, fromY, fromX - 1, awayY, true);
      makeMove(fromX - 1, awayY, toX, toY, true);
      return;
    }

    if (fromX != 8 && !isPlaceOccupied(fromX + 1, awayY)) {
      Serial.println("Front in right");
      makeMove(fromX, fromY, fromX + 1, awayY, true);
      makeMove(fromX + 1, awayY, toX, toY, true);
      return;
    }

    if (awayPawnY > 0 && awayPawnY < 9) {
      Serial.println("moving pawn away");
      // Otherwise move pawn away
      makeMove(fromX, fromY, fromX, awayY, false);
      makeMove(fromX, awayY, fromX, awayPawnY, true);

      // Move horse to place
      makeMove(fromX, awayPawnY, fromX, fromY, false);
      makeMove(fromX, fromY, toX, toY, true);

      // Move pawn back to place
      makeMove(toX, toY, fromX, awayPawnY, false);
      makeMove(fromX, awayPawnY, fromX, awayY, true);
    }
  } else if (abs(fromX - toX) == 2) {
    // Horizontal horse movement
    Serial.println("horse movement horizontal");

    int awayX = fromX - 1;
    int awayPawnX = fromX - 3;
    if (fromX < toX) {
      awayX = fromX + 1;
      awayPawnX = fromX + 3;
    }

    if (!isPlaceOccupied(awayX, fromY)) {
      // Empty next to him
      Serial.println("Horse bellow");
      makeMove(fromX, fromY, awayX, fromY, true);
      makeMove(awayX, fromY, toX, toY, true);
      return;
    }

    if (fromY != 0 && !isPlaceOccupied(awayX, fromY + 1)) {
      makeMove(fromX, fromY, awayX, fromY + 1, true);
      makeMove(awayX, fromY + 1, toX, toY, true);
      return;
    }

    if (fromY != 8 && !isPlaceOccupied(awayX, fromY - 1)) {
      Serial.println("Horse bellow 2.");
      makeMove(fromX, fromY, awayX, fromY - 1, true);
      makeMove(awayX, fromY - 1, toX, toY, true);
      return;
    }

    if (awayPawnX > 0 && awayPawnX < 9) {
      // Otherwise move pawn away
      makeMove(fromX, fromY, awayX, fromY, false);
      makeMove(awayX, fromY, awayPawnX, fromY, true);

      // Move horse to place
      makeMove(awayPawnX, fromY, fromX, fromY, false);
      makeMove(fromX, fromY, toX, toY, true);

      // Move pawn back to place
      makeMove(toX, toY, awayPawnX, fromY, false);
      makeMove(awayPawnX, fromY, awayX, fromY, true);
    }
  }
}