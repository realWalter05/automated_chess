#include "Micro_Max.h"
#include <AccelStepper.h>

// Detect values
char move[4];
char letterTranslate[] = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h'}; 

//  MicroMax
extern char lastH[], lastM[];

// Electromagnet
const int magnetPin = 13;
int magnetActivated = HIGH;

// Motors
#define motorInterfaceType 1

const int stepPinX = 12;
const int dirPinX = 11;
AccelStepper motorX(motorInterfaceType, stepPinX, dirPinX);

const int stepPinY = 10;
const int dirPinY = 9;
AccelStepper motorY(motorInterfaceType, stepPinY, dirPinY);

// Control multiplexor information
bool playingAsWhite = true;
bool userTurn = true;
int difficulty = 0;

// General information
int boxLengthX = 125;
int boxLengthY = 120;
bool gameStarted = false;

int boardValues[8][8];
int boardValuesMemory[8][8];
int recordedReedValue[8];
int controlValues[8];

// Multilexers
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

  motorX.setMaxSpeed(400);
  motorY.setMaxSpeed(400);

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
  if (motorX.distanceToGo() != 0 || motorY.distanceToGo() != 0) {
    motorX.runSpeedToPosition();
    motorY.runSpeedToPosition();
    return;
  }

  if (!gameStarted) {
    // Waiting for user to put pieces in the control part of the board

    // Check control multiplexor
    setControlMUX();
    return;
  }

  // GAME STARTED

  if (userTurn) {
    // Player's turn => waiting for movement
    while (!move[0] && !move[1] && !move[2] && !move[3]) {
      detectBoardMovement();
    }

    strcpy(lastH, move);
    Serial.println("User move: ");
    Serial.println(lastH[0], lastH[1], lastH[2], lastH[3]);

    userTurn = false;

  } else {
    // AI turn
    Serial.println("Ai playing...");
    AI_HvsC(move);

    // TODO make the move
    for (int i = 0; i < 3; i++) {
      if (i == 0) {
        moveMagnet(8, 2, false);
      } else if (i == 1) {
        moveMagnet(6, 2, true);
      } else if (i == 2) {
        moveMagnet(2, 2, true);
      } else if (i == 3) {
        moveMagnet(4, 2, false);
      }
    }

    Serial.println("Ai move: ");
    Serial.println(lastM[0], lastM[1], lastM[2], lastM[3]);

    userTurn = true;
  }
}


void getReedValues(int targetMuxAddress, int from, int to) {
  // DEBUG information
  Serial.println("MUX ");
  Serial.print(targetMuxAddress);
  Serial.print(":    ");

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
    Serial.print(reedValue);

    muxValues[j] = reedValue;

  }

  // Resetting MUX
  digitalWrite(muxAddr[0], LOW);
  digitalWrite(muxAddr[1], LOW);
  digitalWrite(muxAddr[2], LOW);
  digitalWrite(muxAddr[3], LOW);

  digitalWrite(targetMuxAddress, HIGH);
  memcpy(recordedReedValue[8], muxValues, sizeof(recordedReedValue[8]));
}

void setControlMUX() {
  getReedValues(muxEnable[4], 0, 8);
  memcpy(controlValues[8], recordedReedValue, sizeof(controlValues[8]));

  // Setting the data based on MUX wiring
  if (!controlValues[1]) {
    playingAsWhite = false;
  }

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
  }

  if ((!controlValues[1] || !controlValues[2]) &&
      (!controlValues[3] || !controlValues[4] || !controlValues[5])) {
    // Checking if mandatory user info is set
    // TODO check if chess pieces are there
    gameStarted = true;
  }
}

void setCurrentBoard() {
  for (int i = 0; i < 4; i = i + 2) {
      getReedValues(muxEnable[i], 0, 8);
      memcpy(boardValues[i][8], recordedReedValue, sizeof(boardValues[i][8]));

      getReedValues(muxEnable[i], 8, 16);
      memcpy(boardValues[i+1][8], recordedReedValue, sizeof(boardValues[i][8]));      
  }
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
    motorY.move(boxLengthY * distance);
    motorX.move(boxLengthX * distance);
    motorY.setSpeed(350);
    motorX.setSpeed(400);
    break;

  case 2:
    // Down
    motorY.move(-boxLengthY * distance);
    motorY.setSpeed(350);
    break;

  case 3:
    // Diagonal right down
    motorY.move(-boxLengthY * distance);
    motorX.move(-boxLengthX * distance);
    motorY.setSpeed(350);
    motorX.setSpeed(400);
    break;

  case 4:
    // Left
    motorX.move(boxLengthX * distance);
    motorX.setSpeed(400);
    break;

  case 6:
    // Right
    motorX.move(-boxLengthX * distance);
    motorX.setSpeed(400);
    break;

  case 7:
    // Diagonal left up
    motorY.move(-boxLengthY * distance);
    motorX.move(boxLengthX * distance);
    motorY.setSpeed(350);
    motorX.setSpeed(400);
    break;

  case 8:
    // Up
    motorY.move(boxLengthY * distance);
    motorY.setSpeed(350);
    break;

  case 9:
    // Diagonal right up
    motorY.move(boxLengthY * distance);
    motorX.move(-boxLengthX * distance);
    motorY.setSpeed(350);
    motorX.setSpeed(400);
    break;
  }

  // Disabling the electromagnet
  if (magnetActivated) {
    digitalWrite(magnetPin, LOW);
  }
}

void detectBoardMovement() {
  delay(1000);
  memcpy(boardValues[8][8], boardValuesMemory, sizeof(boardValues[8][8]));
  setCurrentBoard();
  
  for (int i = 0; i < 8; i++) {
    for (int j = 0; j < 8; j++) {
      if (boardValuesMemory[i][j] != boardValues[i][j]) {
        // Position of piece has changes
        if (boardValues[i][j]) {
          // Piece was here before
          move[0] = j++;
          move[1] = letterTranslate[j];
        } else {
          // New piece on this position
          move[2] = j++;
          move[3] = letterTranslate[j];
        }
      }
    }
  }
  delay(1000);
}