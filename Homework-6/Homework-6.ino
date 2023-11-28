/*
  8x8 matrix game

  Description:
  This assignment focuses on developing a small game on the 8x8 matrix.
  The game have 3 types of elements: player (blinks slowly),  bombs(blinks fast),
  wall (doesn’tblink). The basic idea is that we generate walls on the map (50% - 75% of the map)
  and we move around with the player and destroy them.

  Components:
    • one joystick
    • one 8x8 led matrix
    • one MAX7219
    • one button
    • one buzzer  
    • resistors and wires as needed

  Created 28-11-2023
  Coriciuc Tudor 

  https://youtu.be/9Z7CiHrCli8
*/



// We need the library library to control the matrix
#include "LedControl.h"   

// We initialize the pins
const int pinX = A0;
const int pinY = A1;
const int buzzerPin = 13;
const int dinPin = 12;    
const int clockPin = 11;  
const int loadPin = 10;   
const int bombBottonPin = 2;

// 
LedControl lc = LedControl(dinPin, clockPin, loadPin, 1);  //DIN, CLK, LOAD, No. DRIVER
byte matrixBrightness = 2;

// 
const byte matrixSize = 8;
int matrix[matrixSize][matrixSize] = {
  { 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0 },
};

// Joystick directions
int xValue = 0;
int yValue = 0;

// We use variables(joyMoved and min/maxThreshold) to make sure we get a correct input from the joystick
bool joyMoved = false;
const int minInputThreshold = 200;
const int maxInputThreshold = 800;
const int minIdleThreshold = 500;
const int maxIdleThreshold = 550;

// Player position
int playerX;
int playerY;

// Player alive status
bool playerAlive = true;

// Bomb variables
bool activeBomb = false;
bool putBomb = false;
int bombX;
int bombY;

// Blink variables
const int playerBlinkDelay = 500;
const int bombBlinkDelay = 200;
unsigned long lastBlinkPlayer = 0;
unsigned long lastBlinkBomb = 0;
bool playerState = 1;
bool bombState = 1;

// The bombs explodes 2 seconds after it was placed down
const int explosionDelay = 2000;
unsigned long bombCounter = 0;

// Game message status
bool shown = false;

// Valid player spawn variables
bool validSpawn = false;
const int minSafeDistance = 2;

// Button vatiables 
bool stateReading;
const int inputDebounceDelay = 100;
unsigned long lastClick = 0;

// Interrupt variables for the button
volatile bool possiblePress = false;
volatile bool shouldStartCounter = false;

// Buzzer variables
const int buzzerTone = 1000;
const int buzzerDuration = 750;

#define SPACE 0
#define WALL 1
#define PLAYER 2
#define BOMB 3

const float fillPercentage = 0.7;

void setup() {
  pinMode(bombBottonPin, INPUT_PULLUP);

  lc.shutdown(0, false);
  lc.setIntensity(0, matrixBrightness);
  lc.clearDisplay(0);
  Serial.begin(9600);
  attachInterrupt(digitalPinToInterrupt(bombBottonPin), handleInterrupt, FALLING);

  // We generate a new random map each time in the setup
  generateMap();
}

void loop() {
  // If the player dies the games ends
  if (playerAlive == true) {
    // We render the game
    renderDisplay();
    // Player game logic 
    player();
    // Exposion
    explosion();
  } else {
    if (!shown) {
      Serial.println("Game over!");
    }
    shown = true;
  }
}

void player() {
  // We place a bomb if the button was pressed and the other conditions are met
  bomb();
  // Player movement
  move();
}

void move() {
  // We read the Joystick inputs and move the player
  xValue = analogRead(pinX);
  yValue = analogRead(pinY);
  if (joyMoved == false) {
    if (xValue < minInputThreshold) {
      // We verify that the player remains in the matrix
      if (matrix[playerX - 1][playerY] == SPACE && playerX - 1 >= 0) {
        // If the button is pressed we place a bomb behind us 
        if (putBomb == true) {
          matrix[playerX][playerY] = BOMB;
          bombCounter = millis();
          activeBomb = true;
          putBomb = false;
          bombX = playerX;
          bombY = playerY;
        } else {
          matrix[playerX][playerY] = SPACE;
        }
        playerX--;
        matrix[playerX][playerY] = PLAYER;
        // The player led is ON on the next position
        playerBlink(playerX, playerY, true);
      }
      joyMoved = true;
    } else if (xValue > maxInputThreshold) {
      if (matrix[playerX + 1][playerY] == SPACE && playerX + 1 < matrixSize) {
        if (putBomb == true) {
          matrix[playerX][playerY] = BOMB;
          bombCounter = millis();
          activeBomb = true;
          putBomb = false;
          bombX = playerX;
          bombY = playerY;
        } else {
          matrix[playerX][playerY] = SPACE;
        }
        playerX++;
        matrix[playerX][playerY] = PLAYER;
        playerBlink(playerX, playerY, true);
      }
      joyMoved = true;
    } else if (yValue < minInputThreshold) {
      if (matrix[playerX][playerY - 1] == SPACE && playerY - 1 >= 0) {
        if (putBomb == true) {
          matrix[playerX][playerY] = BOMB;
          bombCounter = millis();
          activeBomb = true;
          putBomb = false;
          bombX = playerX;
          bombY = playerY;
        } else {
          matrix[playerX][playerY] = SPACE;
        }
        playerY--;
        matrix[playerX][playerY] = PLAYER;
        playerBlink(playerX, playerY, true);
      }
      joyMoved = true;
    } else if (yValue > maxInputThreshold) {
      if (matrix[playerX][playerY + 1] == SPACE && playerY + 1 < matrixSize) {
        if (putBomb == true) {
          matrix[playerX][playerY] = BOMB;
          bombCounter = millis();
          activeBomb = true;
          putBomb = false;
          bombX = playerX;
          bombY = playerY;
        } else {
          matrix[playerX][playerY] = SPACE;
        }
        playerY++;
        matrix[playerX][playerY] = PLAYER;
        playerBlink(playerX, playerY, true);
      }
      // We mark that the joystick moved
      joyMoved = true;
    }
    // If the joystick is in the idle position again we can make a new move after
  } else if ((xValue >= minIdleThreshold && xValue <= maxIdleThreshold)
             && (yValue >= minIdleThreshold && yValue <= maxIdleThreshold)) {
    joyMoved = false;
  }
}

void bomb() {
  // 
  if (possiblePress) {
    if (shouldStartCounter) {
      lastClick = millis();
      shouldStartCounter = false;
    }
    if ((millis() - lastClick) > inputDebounceDelay) {
      stateReading = digitalRead(bombBottonPin);
      if (stateReading == LOW && activeBomb == false) {
        putBomb = true;
      }
      possiblePress = false;
    }
  }
}

void renderDisplay() {
  // We go through each tile of the matrix and set it accordingly
  for (int i = 0; i < matrixSize; i++) {
    for (int j = 0; j < matrixSize; j++) {
      switch (matrix[i][j]) {
        case SPACE:
          lc.setLed(0, i, j, 0);
          break;
        case WALL:
          lc.setLed(0, i, j, 1);
          break;
        case PLAYER:
        // The player blinks
          if (millis() - lastBlinkPlayer >= playerBlinkDelay) {
            playerBlink(i, j, false);
          }
          break;
        case BOMB:
        // The bomb blinks
          if (millis() - lastBlinkBomb >= bombBlinkDelay) {
            bombState = !bombState;
            lc.setLed(0, i, j, bombState);
            lastBlinkBomb = millis();
          }
        default:
          break;
      }
    }
  }
}

void playerBlink(int i, int j, bool moved) {
  // If the player moves the led is ON in order to be more visible
  if (moved == true) {
    playerState = true;
  } else {
  // else we change the change its state
    playerState = !playerState;
  }
  lc.setLed(0, i, j, playerState);
  lastBlinkPlayer = millis();
}


void explosion() {
  // The bomb explodes after 2 seconds if it is active
  if (millis() - bombCounter >= explosionDelay && activeBomb == true) {
    // It explodes in a 3X3 square area
    for (int explosionRadiusX = -1; explosionRadiusX <= 1; explosionRadiusX++) {
      for (int explosionRadiusY = -1; explosionRadiusY <= 1; explosionRadiusY++) {
        // We make sure that the bomb cant explode the other side of the map if it is placed near an edge
        if (bombX + explosionRadiusX < matrixSize && bombX + explosionRadiusX >= 0 && bombY + explosionRadiusY < matrixSize && bombY + explosionRadiusY >= 0) {
          if (matrix[bombX + explosionRadiusX][bombY + explosionRadiusY] == PLAYER) {
            // If the player is in the radius of the bomb he dies and its game over
            playerAlive = false;
          }
          // We destory all the tiles in the radius
          matrix[bombX + explosionRadiusX][bombY + explosionRadiusY] = SPACE;
        }
      }
    }
    // We mark the explosion with a buzzer sound
    tone(buzzerPin, buzzerTone, buzzerDuration);
    activeBomb = false;
  }
}

void generateMap() {
  // We choose a random seed using an empty analog port
  randomSeed(analogRead(A2));
  // We calculate how many tiles we fill with walls 
  int numElementsToFill = fillPercentage * matrixSize * matrixSize;

  for (int count = 0; count < numElementsToFill;) {
    int randomRow = random(matrixSize);
    int randomCol = random(matrixSize);

    // Check if the element is not already filled
    if (matrix[randomRow][randomCol] == SPACE) {
      matrix[randomRow][randomCol] = WALL;
      count++;
    }
  }

  // We spawn the player and make sure that he can place at least a bomb without dying
  playerX = random(matrixSize);
  playerY = random(matrixSize);
  matrix[playerX][playerY] = PLAYER;
  const int numberOfDirections = 4;
  // In order to do that we make a 2 tiles space in a random direction
  while (!validSpawn) {
    int direction = random(numberOfDirections);
    switch (direction) {
      case 1:
        if (playerX - minSafeDistance >= 0) {
          for (int distance = 1; distance <= minSafeDistance; distance++) {
            matrix[playerX - distance][playerY] = SPACE;
            validSpawn = true;
          }
        }
        break;
      case 2:
        if (playerY - minSafeDistance >= 0) {
          for (int distance = 1; distance <= minSafeDistance; distance++) {
            matrix[playerX][playerY - distance] = SPACE;
            validSpawn = true;
          }
        }
        break;
      case 3:
        if (playerX + minSafeDistance < matrixSize) {
          for (int distance = 1; distance <= minSafeDistance; distance++) {
            matrix[playerX + distance][playerY] = SPACE;
            validSpawn = true;
          }
        }
        break;
      case 4:
        if (playerY + minSafeDistance < matrixSize) {
          for (int distance = 1; distance <= minSafeDistance; distance++) {
            matrix[playerX][playerY + distance] = SPACE;
            validSpawn = true;
          }
        }
        break;
    }
  }
}

// Interrupt for the bomb button
void handleInterrupt() {
  possiblePress = true;
  shouldStartCounter = true;
}