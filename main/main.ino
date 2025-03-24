#include "Micro_Max.h"

// Detect values
char move[4] = {0, 0, 0, 0};
char numberTranslate[8] = {'1', '2', '3', '4', '5', '6', '7', '8'};
char letterTranslate[8] = {'h', 'g', 'f', 'e', 'd', 'c', 'b', 'a'};
char aiLetterTranslate[8] = "abcdefgh";

// Electromagnet
const int magnetPin = 13;
bool magnetActivated = LOW;

// Electromagnet positioned on a1
int magnetX = 1;
int magnetY = 1;

// Motors
const int stepPinX = 12;
const int dirPinX = 11;

const int stepPinY = 10;
const int dirPinY = 9;

// Control multiplexor information
bool playingAsWhite = true;
bool userTurn = true;
int difficulty = 0;

// General information
int boxLength = 200;
int motorDelay = 800;
bool gameStarted = false;
int showOffMove = 0;

int boardValues[8][8];
int boardValuesMemory[8][8];
int recordedReedValue[8];
int controlValues[8];

// Multiplexers
int muxEnable[5] = {A0, 4, 5, 6, 7};
int muxAddr[4] = {A1, A2, A3, A4};
int muxOutput = A5;

void setup() {
  // DEBUG info
  Serial.begin(9600);

  // Electromagnet
  pinMode(magnetPin, OUTPUT);

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
  pinMode(muxEnable[4], OUTPUT);
  for (int i = 0; i < 4; i++) {
    pinMode(muxEnable[i], OUTPUT);
    digitalWrite(muxEnable[i], HIGH);
  }

  for (int i = 0; i < 4; i++) {
    pinMode(muxAddr[i], OUTPUT);
    digitalWrite(muxAddr[i], LOW);
  }
}

void loop() {
  if (!gameStarted) {
    // Waiting for user to put pieces in the control part of the board

    // Check control multiplexor
    setControlMUX();
    return;
  }

  // GAME STARTED

  if (userTurn) {
    // Player's turn => waiting for movement
    memcpy(boardValuesMemory, boardValues, sizeof(boardValuesMemory));
    while (!move[0] || !move[1] || !move[2] || !move[3]) {
      detectBoardMovement();
    }

    // Debug info
    Serial.print("User: ");
    Serial.print(move[0]);
    Serial.print(move[1]);
    Serial.print(" -> ");
    Serial.println(move[2]);
    Serial.print(move[3]);

    delay(3000);

    // AI turn
    Serial.println("Ai playing...");
    getAIMove(move);

    if (!validMove) {
      // If user made invalid move
      // Resseting move
      move[0] = 0;
      move[1] = 0;
      move[2] = 0;
      move[3] = 0;
      return;
    }

    // Make Ai's move stored in lastMoveAI
    makeChessMove(lastMoveAI);

    // Resseting move
    move[0] = 0;
    move[1] = 0;
    move[2] = 0;
    move[3] = 0;
    delay(3000);
  }
}

void getReedValues(int targetMuxAddress, int from, int to) {
  // Splitting to two rows because row of chessboard has 8 pieces
  int muxValues[8];

  // Activating MUX
  digitalWrite(targetMuxAddress, LOW);

  // TODO otočit pořadí a otestovat
  for (int j = from; j < to; j++) {
    // Checking MUX combinations
    digitalWrite(muxAddr[0], j % 2);
    digitalWrite(muxAddr[1], j / 2 % 2);
    digitalWrite(muxAddr[2], j / 4 % 2);
    digitalWrite(muxAddr[3], j / 8 % 2);

    int reedValue = digitalRead(muxOutput);

    if (j > 7) {
      muxValues[7 - (j - 8)] = reedValue;
    } else {
      muxValues[j] = reedValue;
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

void setControlMUX() {
  getReedValues(muxEnable[4], 0, 8);
  delay(100);
  memcpy(controlValues, recordedReedValue, sizeof(controlValues));

  // Setting the data based on MUX wiring
  if (!controlValues[1]) {
    playingAsWhite = false;
  }
  /*
  if (!controlValues[2]) {
    playingAsWhite = true;
  }

  if (!controlValues[3]) {
    difficulty = 0;
  }

  if (!controlValues[4]) {
    difficulty = 1;
  }

  if (!controlValues[5]) {
    difficulty = 2;
  }*/

  //  if ((!controlValues[1] || !controlValues[2]) &&
  //    (!controlValues[3] || !controlValues[4] || !controlValues[5])) {
  // Checking if mandatory user info is set
  // TODO check if chess pieces are there
  Serial.println("Game started");
  gameStarted = true;
  setCurrentBoard();
}

void moveMagnet(int direction, int distance, bool magnetActivated) {
  // Enabling the electromagnet if neccesary
  if (magnetActivated) {
    digitalWrite(magnetPin, HIGH);
  }

  switch (direction) {
  // Direction of the movement based on position of key in numeric keypad
  case 1:
    // Diagonal left down
    digitalWrite(dirPinY, LOW);
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
    digitalWrite(dirPinY, HIGH);
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
  digitalWrite(magnetPin, LOW);
  digitalWrite(dirPinX, LOW);
  digitalWrite(dirPinY, LOW);
  digitalWrite(stepPinX, LOW);
  digitalWrite(stepPinY, LOW);
}

void detectBoardMovement() {
  setCurrentBoard();

  Serial.println("////// Board //////");
  for (int i = 7; i >= 0; i--) {
    Serial.print(i + 1);
    Serial.print(" / ");
    for (int j = 7; j >= 0; j--) {
      Serial.print(boardValues[i][j]);
      Serial.print(" ");
    }
    Serial.println();
  }
  Serial.println("    A B C D E F G H");

  Serial.println("");

  for (int i = 0; i < 8; i++) {
    for (int j = 0; j < 8; j++) {
      if (boardValuesMemory[i][j] != boardValues[i][j]) {
        // Position of piece has changes
        if (boardValues[i][j] && !move[0] && !move[1]) {
          // Piece was here before
          move[0] = letterTranslate[j];
          move[1] = numberTranslate[i];
        } else if (!boardValues[i][j] && !move[2] && !move[3]) {
          // New piece on this position
          move[2] = letterTranslate[j];
          move[3] = numberTranslate[i];
        }
      }
    }
  }
  delay(2000);
}

void makeChessMove(char givenMove[5]) {
  // Deconstruct move
  Serial.println("making chess move");
  Serial.println(givenMove);
  int fromX = strchr(aiLetterTranslate, givenMove[0]) - aiLetterTranslate + 1;
  int fromY = givenMove[1] - '0';

  int toX = strchr(aiLetterTranslate, givenMove[2]) - aiLetterTranslate + 1;
  int toY = givenMove[3] - '0';

  Serial.println("Moving magnet from: ");
  Serial.print(fromX);
  Serial.print(fromY);
  Serial.print("->");
  Serial.print(toX);
  Serial.println(toY);
  // TODO if we are taking a piece

  // Move magnet to starting position on X
  if (fromX > magnetX) {
    // Move to right
    moveMagnet(6, fromX - magnetX, false);
  } else if (fromX < magnetX) {
    // Move to left
    moveMagnet(4, magnetX - fromX, false);
  }

  // Move magnet to starting position on Y
  if (fromY > magnetY) {
    // Move up
    moveMagnet(8, fromY - magnetY, false);
  } else if (fromY < magnetY) {
    // Move down
    moveMagnet(2, magnetY - fromY, false);
  }

  Serial.println("Magnet is on");
  Serial.print(magnetX);
  Serial.println(magnetY);

  // Move magnet with piece
  if (toX > magnetX) {
    // Move to right
    moveMagnet(6, toX - magnetX, true);
  } else if (toX < magnetX) {
    // Move to left
    moveMagnet(4, magnetX - toX, true);
  }

  // Move magnet to starting position on Y
  if (toY > magnetY) {
    // Move up
    moveMagnet(8, toY - magnetY, true);
  } else if (toY < magnetY) {
    // Move down
    moveMagnet(2, magnetY - toY, true);
  }

  Serial.println("Magnet is on");
  Serial.print(magnetX);
  Serial.println(magnetY);

  /*
    vež může jenom rovně a do stran
    střelec může jenom diagonálně
    kůň batshit crazy
    dáma na všechny strany
    král na všechny strany o jeden, a ještě umí castlovat
    pěšec, takuje do stran, en passant a na konci dela cavyky
  */
}