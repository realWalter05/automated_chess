#include <AccelStepper.h>

// Define pin connections & motor's steps per revolution
const int directionY = 11;
const int directionX = 9;
const int motorYPin = 10;
const int motorXPin = 8;

const int transistorPin = 13;
int transistorValue = HIGH;

// Define motor interface type
#define motorInterfaceType 1

// Creates an instance
AccelStepper motorY(motorInterfaceType, motorYPin, directionY);
AccelStepper motorX(motorInterfaceType, motorXPin, directionX);

void setup() {
  pinMode(transistorPin, OUTPUT);
  
  motorX.setMaxSpeed(400);
  motorY.setMaxSpeed(400);

  Serial.begin(9600);
  Serial.println("Where to move magnet (1235) or control transistor 9(on)-0(off): ");
}

void loop() {
    // Control read
    if (motorX.distanceToGo() == 0 && motorY.distanceToGo() == 0) {

      int menuChoice = Serial.parseInt();
      Serial.println(menuChoice);

      switch (menuChoice) {
        case 1:
          motorX.move(100);
          motorX.setSpeed(400);

          break;

        case 2:
          motorY.move(-100);
          motorY.setSpeed(150);

          break;

        case 3:
          motorX.move(-100);
          motorX.setSpeed(400);
          break;

        case 4:
          motorY.move(-100);
          motorX.move(100);
          motorY.setSpeed(150);
          motorX.setSpeed(400);

          break;

        case 5:
          motorY.move(150);
          motorY.setSpeed(200);
          break;

        case 6:
          motorY.move(-100);
          motorX.move(-100);
          motorY.setSpeed(150);
          motorX.setSpeed(400);

          break;

        case 9:
          transistorValue = HIGH;
          break;


        case 8:
          transistorValue = LOW;
          break;

        default:
          Serial.println("Please choose a valid selection...");
      }
    }
    digitalWrite(transistorPin, transistorValue);  
    motorX.runSpeedToPosition();
    motorY.runSpeedToPosition();
}