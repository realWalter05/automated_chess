#include "global.h"
#include "Micro_Max.h"
#include <AccelStepper.h>

// Electromagnet
const int magnetPin = 13;
const int magnetPin1 = 0;
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
bool resetPieces = false;
bool difficulty = 0;

// General information
int boxLengthX = 125;
int boxLengthY = 120;
bool gameStarted = false;
int boardValues [2][16];
int showOff = 0;


// Multilexers
int muxEnable[4] = {4, 0, 0, 0};
int muxAddr[4] = {A0, A1, A2, A3};
int muxOutput = 2;

void setup() {
	// Electromagnet
	pinMode(magnetPin, OUTPUT);
	pinMode(magnetPin1, OUTPUT);
  
	// Motors
  pinMode(stepPinX, OUTPUT);
	pinMode(dirPinX, OUTPUT);
	pinMode(stepPinY, OUTPUT);
  pinMode(dirPinY, OUTPUT);

  motorX.setMaxSpeed(400);
  motorY.setMaxSpeed(400);

  // Setting multiplexer output as Input_pullup
	pinMode(muxOutput, INPUT_PULLUP);

  // Disabling Multiplexers  (When high - does nothing)
  for (int i = 0; i < 4; i++) {
    pinMode(muxEnable[i], OUTPUT);
    digitalWrite(muxEnable[i], HIGH);
  }

  for(int i = 0; i < 4; i++) {
    pinMode(muxAddr[i], OUTPUT);
    digitalWrite(muxAddr[i], LOW);
  }
  Serial.begin(9600);
}



void loop() {
  digitalWrite(magnetPin, HIGH);
  digitalWrite(magnetPin1, LOW);
  if (!gameStarted) {
    // Waiting for user to put pieces in the control part of the board
    
    // Check control multiplexor
	  checkControlMultiplexor();
    return;
  }

  if (motorX.distanceToGo() != 0 || motorY.distanceToGo() != 0) {
    motorX.runSpeedToPosition();
    motorY.runSpeedToPosition();
    return;
  }/*

	if (playingAsWhite) {
		// White player's turn => waiting for movement
    checkReeds();
    if (boardValues[0][0] == 0) {
      moveMotor(2, 4);
      //moveMotor(2, 4);
    } else if (boardValues[0][15] == 0) {
      moveMotor(4, 4);
      //moveMotor(4, 4);
    }
	} else {
		// Black player's turn
    Serial.println("Ai is black...");
    AI_HvsC();
	}*/

  // Show off
  switch (showOff) {
    case 0:
      moveMotor(8, 4);
      break;
    case 1:
      moveMotor(6, 6);
      break;
    case 2:
      moveMotor(8, 4);
      break;
    case 3:
      moveMotor(4, 6);
      break;
    case 4:
      moveMotor(2, 4);
      break;           
    case 5:
      moveMotor(6, 8);
      break;
    case 6:
      moveMotor(2, 4);
      break;
    case 7:
      moveMotor(4, 8);
      break;
    case 8: 
      moveMotor(9, 8);
      break;
    case 9:
      moveMotor(4, 8);
      break;
    case 10:
      moveMotor(3, 8);
      break;
    case 11: 
      moveMotor(4, 8);
    break;
  }

  showOff = showOff + 1;
}



void moveMotor(int direction, int distance) {
  switch (direction) {
    // Direction of the movement based on position of key in numeric keypad
    case 1:
      // Diagonal left down
      motorY.move(boxLengthY*distance);;
      motorX.move(boxLengthX*distance);;
      motorY.setSpeed(350);
      motorX.setSpeed(400);
      break;

    case 2:
      // Down
      motorY.move(-boxLengthY*distance);;
      motorY.setSpeed(350);
      break;

    case 3:
      // Diagonal right down
      motorY.move(-boxLengthY*distance);;
      motorX.move(-boxLengthX*distance);;
      motorY.setSpeed(350);
      motorX.setSpeed(400);
      break;

    case 4:
      // Left
      motorX.move(boxLengthX*distance);
      motorX.setSpeed(400);
      break;

    case 6:
      // Right
      motorX.move(-boxLengthX*distance);;
      motorX.setSpeed(400);
      break;

    case 7:
      // Diagonal left up
      motorY.move(-boxLengthY*distance);;
      motorX.move(boxLengthX*distance);
      motorY.setSpeed(350);
      motorX.setSpeed(400);
      break;

    case 8:
      // Up
      motorY.move(boxLengthY*distance);
      motorY.setSpeed(350);
      break;

    case 9:
      // Diagonal right up
      motorY.move(boxLengthY*distance);;
      motorX.move(-boxLengthX*distance);;
      motorY.setSpeed(350);
      motorX.setSpeed(400);
      break;
  }
  
  // Enabling the electromagnet
  //digitalWrite(magnetPin, magnetActivated);  

  // Running the motors
  //motorX.runSpeedToPosition();
  //motorY.runSpeedToPosition();

  // Disabling the electromagnet
  //digitalWrite(magnetPin, magnetActivated);  
}



void checkControlMultiplexor() {
	// TODO
	gameStarted = true;
	difficulty = 1;
	playingAsWhite = true;
	resetPieces = false;
}

void checkReeds() {
  for(int i = 0; i < 2; i++) {
      int currentMux = muxEnable[i];
      digitalWrite(currentMux, LOW);

      for(int j = 0; j < 16; j++) {
        // Checking all combinations
        digitalWrite(muxAddr[0], j%2);
        digitalWrite(muxAddr[1], j/2%2);
        digitalWrite(muxAddr[2], j/4%2);
        digitalWrite(muxAddr[3], j/8%2);

        int reedValue = digitalRead(muxOutput);
        boardValues[i][j] = reedValue;

      }
      digitalWrite(muxAddr[0], LOW);
      digitalWrite(muxAddr[1], LOW);
      digitalWrite(muxAddr[2], LOW);
      digitalWrite(muxAddr[3], LOW);

      digitalWrite(currentMux, HIGH);
  }
}