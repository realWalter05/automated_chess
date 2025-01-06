#include "global.h"
#include "Micro_Max.h"
#include <AccelStepper.h>

// Electromagnet
const int magnetPin = 13;
int magnetActivated = HIGH;

// Motors
const int stepPinX = A0;
const int dirPinx = A1;
AccelStepper motorX(1, stepPinX, dirPinx);

const int stepPinY = A2;
const int dirPinY = A3;
AccelStepper motorY(1, stepPinY, dirPinY);

// Control multiplexor information
bool playingAsWhite = true;
bool resetPieces = false;
bool difficulty = 0;

// General information
int boxLength = 100;
bool gameStarted = false;

// Multilexers
int muxEnable[4] = {4, 5, 0, 0};
int muxAddr[4] = {11, 10, 9, 8};
int muxOutput = 2;

void setup() {
	// Electromagnet
	pinMode(magnetPin, OUTPUT);
  
	// Motors
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


  // Serial communication for testing purposes
  Serial.begin(9600);
}



void loop() {
  if (!gameStarted) {
    Serial.println("Game has not started yet");
    // Waiting for user to put pieces in the control part of the board
    
    // Check control multiplexor
	  checkControlMultiplexor();
    return;
  }
  
	if (playingAsWhite) {
		// White player's turn => waiting for movement
    Serial.println("Printing reeds.");
    checkReeds();
	} else {
		// Black player's turn
    Serial.println("Ai is black...");
    AI_HvsC();
	}
  
  // Wait ten seconds
  delay(10000);
}



void moveMotor(int direction, int distance) {
  switch (direction) {
    // Direction of the movement based on position of key in numeric keypad
    case 1:
      // Diagonal left down
      motorY.move(boxLength*distance);;
      motorX.move(boxLength*distance);;
      motorY.setSpeed(150);
      motorX.setSpeed(400);
      break;

    case 2:
      // Down
      motorY.move(-boxLength*distance);;
      motorY.setSpeed(150);
      break;

    case 3:
      // Diagonal right down
      motorY.move(boxLength*distance);;
      motorX.move(-boxLength*distance);;
      motorY.setSpeed(150);
      motorX.setSpeed(400);
      break;

    case 4:
      // Left
      motorX.move(boxLength*distance);
      motorX.setSpeed(400);
      break;

    case 6:
      // Right
      motorX.move(-boxLength*distance);;
      motorX.setSpeed(400);
      break;

    case 7:
      // Diagonal left up
      motorY.move(-boxLength*distance);;
      motorX.move(boxLength*distance);
      motorY.setSpeed(150);
      motorX.setSpeed(400);
      break;

    case 8:
      // Up
      motorY.move(150);
      motorY.setSpeed(200);
      break;

    case 9:
      // Diagonal right up
      motorY.move(-boxLength*distance);;
      motorX.move(-boxLength*distance);;
      motorY.setSpeed(150);
      motorX.setSpeed(400);
      break;
  }
  
  // Enabling the electromagnet
  digitalWrite(magnetPin, magnetActivated);  

  // Running the motors
  motorX.runSpeedToPosition();
  motorY.runSpeedToPosition();

  // Disabling the electromagnet
  digitalWrite(magnetPin, magnetActivated);  
}



void checkControlMultiplexor() {
  Serial.println("Checking control multiplexor");
	// TODO
	gameStarted = true;
	difficulty = 1;
	playingAsWhite = true;
	resetPieces = false;
}

void checkReeds() {
  for(int i = 0; i < 2; i++) {
      Serial.print("Checking reeds for mux: ");
      Serial.println(i);

      int currentMux = muxEnable[i];
      Serial.println(currentMux);
      digitalWrite(currentMux, LOW);

      for(int j = 0; j < 16; j++) {
        // Checking all combinations
        Serial.print(j);  
        Serial.print(" - ");
        Serial.print(j/8%2);
        Serial.print(j/4%2);
        Serial.print(j/2%2);
        Serial.print(j%2);
        Serial.print(" - ");
        
        digitalWrite(muxAddr[0], j%2);
        digitalWrite(muxAddr[1], j/2%2);
        digitalWrite(muxAddr[2], j/4%2);
        digitalWrite(muxAddr[3], j/8%2);

        int reedValue = digitalRead(muxOutput);
        Serial.println(reedValue);

      }
      digitalWrite(muxAddr[0], LOW);
      digitalWrite(muxAddr[1], LOW);
      digitalWrite(muxAddr[2], LOW);
      digitalWrite(muxAddr[3], LOW);

      digitalWrite(currentMux, HIGH);
  }
}