#include "Micro_Max.h"
#include "global.h"
#include <AccelStepper.h>

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
  checkReeds();

  if (userTurn) {
    // Player's turn => waiting for movement


  } else {
    // AI turn
    Serial.println("Ai is black...");
    AI_HvsC();
  }
}

void setControlMUX() {
  controlValues = getReedValues(muxEnable[5])[0];

  // Setting the data based on MUX wiring
  if (!controlValues[1]) {
    playingAsWhite = false;
  }

  if (!controlValues[2]) {
    playingAsWhite = true;
  }

  if (!controlValues[3]) {
    difficulty = 0
  }

  if (!controlValues[4]) {
    difficulty = 1
  }

  if (!controlValues[5]) {
    difficulty = 2
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
    int currentReedValues[2] = getReedValues(muxEnable[i]);
    boardValues[i] = currentReedValues[0];
    boardValues[i+1] = currentReedValues[1];
  }
}

void getReedValues(int targetMuxAddress) {
  // DEBUG information
  Serial.println("MUX ");
  Serial.print(i);
  Serial.print(":    ");

  // Splitting to two rows because row of chessboard has 8 pieces
  int muxValuesFirstRow[8];
  int muxValuesSecondRow[8];

  // Activating MUX
  digitalWrite(targetMuxAddress, LOW);

  for (int j = 0; j < 16; j++) {
    // Checking MUX combinations
    digitalWrite(muxAddr[0], j % 2);
    digitalWrite(muxAddr[1], j / 2 % 2);
    digitalWrite(muxAddr[2], j / 4 % 2);
    digitalWrite(muxAddr[3], j / 8 % 2);

    int reedValue = digitalRead(muxOutput);
    Serial.print(reedValue);

    if (j < 8) {
      muxValues[j] = reedValue;
    } else {
      muxValuesSecondRow[j-8] = reedValue;
    }
  }

  // Resetting MUX
  digitalWrite(muxAddr[0], LOW);
  digitalWrite(muxAddr[1], LOW);
  digitalWrite(muxAddr[2], LOW);
  digitalWrite(muxAddr[3], LOW);

  digitalWrite(targetMuxAddress, HIGH);
  return {muxValuesFirstRow, muxValuesSecondRow};
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

  int newBoardValues[4][16];

  
  for (int i = 0; i < 8; i++) {
    for (int j = 0; j < 8; j++) {
      if (newBoardValues[i][j] != currentReedValues[i][j]) {
        // Position of piece has changes
        
      }
    }
  }
}