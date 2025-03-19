#include "Micro_Max.h"
#include <AccelStepper.h>

// Detect values
char move[4];
char letterTranslate[] = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h'}; 

//  MicroMax
extern char lastH[], lastM[];

// Electromagnet
const int magnetPin = 13;
bool magnetActivated = LOW;

int showOff = 0;

// Motors
#define motorInterfaceType 1

const int stepPinX = 10;
const int dirPinX = 9;
AccelStepper motorX(motorInterfaceType, stepPinX, dirPinX);

const int stepPinY = 12;
const int dirPinY = 11;
AccelStepper motorY(motorInterfaceType, stepPinY, dirPinY);

// Control multiplexor information
bool playingAsWhite = true;
bool userTurn = true;
int difficulty = 0;

// General information
int boxLengthX = 125;
int boxLengthY = 120;
bool gameStarted = false;
int showOffMove = 0;

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
  if (!gameStarted) {
    // Waiting for user to put pieces in the control part of the board

    // Check control multiplexor
    setControlMUX();
    return;
  }

  while (motorX.distanceToGo() != 0 || motorY.distanceToGo() != 0) {

    motorX.runSpeedToPosition();
    motorY.runSpeedToPosition();
    return;
  }
  // GAME STARTED

  /*if (userTurn) {
    // Player's turn => waiting for movement
    while (!move[0] && !move[1] && !move[2] && !move[3]) {
      detectBoardMovement();
    }

    strcpy(lastH, move);
    Serial.println("User move: ");
    Serial.print(lastH[0]);
    Serial.print(lastH[1]);
    Serial.print(lastH[2]);
    Serial.print(lastH[3]);
    


    userTurn = false;
    motorX.moveTo(-100);
    motorX.setSpeed(400);

  } else {
    // AI turn
    Serial.println("Ai playing...");
    //AI_HvsC(move);

    Serial.println("Ai move: ");
    Serial.print(lastM[0]);
    Serial.print(lastM[1]);
    Serial.print(lastM[2]);
    Serial.print(lastM[3]);


    userTurn = true;
    motorX.moveTo(100);
    motorX.setSpeed(400);
  }
  delay(2500);*/

  // Show off
  switch (showOff) {
    case 0:
      moveMagnet(8, 4, true);
      break;
    case 1:
      moveMagnet(6, 4, true);
      break;
    case 2:
      moveMagnet(2, 4, true);
      break;
    case 3:
      moveMagnet(4, 4, true);
      break;
  }

  showOff++;
}


void getReedValues(int targetMuxAddress, int from, int to) {
  // DEBUG information
  Serial.print("MUX ");
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
  Serial.println("");

  memcpy(recordedReedValue, muxValues, sizeof(recordedReedValue));
}

void setControlMUX() {
  getReedValues(muxEnable[4], 0, 8);
  delay(100);
  memcpy(controlValues, recordedReedValue, sizeof(controlValues));

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
    Serial.println("Game started");
    gameStarted = true;
  }
}

void setCurrentBoard() {
    getReedValues(muxEnable[0], 0, 8);
    delay(100);
    memcpy(boardValues[0], recordedReedValue, sizeof(boardValues[0]));

    getReedValues(muxEnable[0], 8, 16);
    delay(100);
    memcpy(boardValues[1], recordedReedValue, sizeof(boardValues[1]));      

    getReedValues(muxEnable[1], 0, 8);
    delay(100);
    memcpy(boardValues[2], recordedReedValue, sizeof(boardValues[2]));

    getReedValues(muxEnable[1], 8, 16);
    delay(100);
    memcpy(boardValues[3], recordedReedValue, sizeof(boardValues[3]));  

    getReedValues(muxEnable[2], 0, 8);
    delay(100);
    memcpy(boardValues[4], recordedReedValue, sizeof(boardValues[4]));

    getReedValues(muxEnable[2], 8, 16);
    delay(100);
    memcpy(boardValues[5], recordedReedValue, sizeof(boardValues[5]));  

    getReedValues(muxEnable[3], 0, 8);
    delay(100);
    memcpy(boardValues[6], recordedReedValue, sizeof(boardValues[6]));

    getReedValues(muxEnable[3], 8, 16);
    delay(100);
    memcpy(boardValues[7], recordedReedValue, sizeof(boardValues[7]));      
}


void moveMagnet(int direction, int distance, bool magnetActivated) {
  Serial.println("moving magnet");
  // Enabling the electromagnet if neccesary
  if (magnetActivated) {
    digitalWrite(magnetPin, HIGH);
  }

  switch (direction) {
  // Direction of the movement based on position of key in numeric keypad

  case 1:
    // Diagonal left down
    motorY.moveTo(boxLengthY * distance);
    motorX.moveTo(boxLengthX * distance);
    motorY.setSpeed(250);
    motorX.setSpeed(400);
    break;

  case 2:
    // Down
    motorY.moveTo(-boxLengthY * distance);
    motorY.setSpeed(250);
    break;

  case 3:
    // Diagonal right down
    motorY.moveTo(-boxLengthY * distance);
    motorX.moveTo(-boxLengthX * distance);
    motorY.setSpeed(250);
    motorX.setSpeed(300);
    break;

  case 4:
    // Left
    motorX.moveTo(-100);
    motorX.setSpeed(300);
    break;

  case 6:
    // Right
    motorX.moveTo(100);
    motorX.setSpeed(300);
    break;

  case 7:
    // Diagonal left up
    motorY.moveTo(-boxLengthY * distance);
    motorX.moveTo(boxLengthX * distance);
    motorY.setSpeed(250);
    motorX.setSpeed(300);
    break;

  case 8:
    // Up
    motorY.moveTo(boxLengthY * distance);
    motorY.setSpeed(250);
    break;

  case 9:
    // Diagonal right up
    motorY.moveTo(boxLengthY * distance);
    motorX.moveTo(-boxLengthX * distance);
    motorY.setSpeed(250);
    motorX.setSpeed(300);
    break;
  }

  // Disabling the electromagnet
  if (magnetActivated) {
    digitalWrite(magnetPin, LOW);
  }

}

void detectBoardMovement() {
  delay(1000);
  memcpy(boardValues, boardValuesMemory, sizeof(boardValues));
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