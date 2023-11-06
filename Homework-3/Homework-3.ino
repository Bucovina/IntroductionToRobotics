/*
  7 segment display drawing

  Description:
  This assignment focuses on using the joystick to control the position of
  the segment and ”draw” on the display.

  Components:
  • one 7-segment display
  • one joystick
  • one buzzer
  • resistors and wires as needed

  Created 06-11-2023
  Coriciuc Tudor 

  https://youtu.be/L5BbXVPg4Ss
*/

// Declare all the joystick pins
// Digital pin connected to switch output
const int pinSW = 2;
// A0 - Analog pin connected to X output
const int pinX = A0;
// A1 - Analog pin connected to Y output
const int pinY = A1;

// Declare all the segments pins
const int pinA = 12;
const int pinB = 10;
const int pinC = 9;
const int pinD = 8;
const int pinE = 7;
const int pinF = 6;
const int pinG = 5;
const int pinDP = 4;

const int numberOfSegments = 8;
const int firstSegment = 0;
const int DPsegment = 7;
// Directions: Up, Down, Left, Right
const int numberOfDirections = 4;

// Initially the current segment is the DP(the decimal point)
int currentSegment = DPsegment;

bool swState = HIGH;
int xValue = 0;
int yValue = 0;

// We use variables(joyMoved and min/maxThreshold) to make sure we get a correct input from the joystick
bool joyMoved = false;
const int minInputThreshold = 200;
const int maxInputThreshold = 800;
const int minIdleThreshold = 500;
const int maxIdleThreshold = 550;

// In order to indicate the current segment on the display, we make it blink
const int blinkDelay = 500;
unsigned long lastBlink = 0;
// We use blinkState to change the state of the current segment without interfering with the segmentStates
bool blinkState = LOW;

// We use debouncing on the joystick switch
bool stateReading;
const int inputDebounceDelay = 100;
unsigned long lastClick = 0;

// If the hold the button pressed more than 1,5 seconds we reset the segments
const int resetTime = 1500;
unsigned long longPressTime = 0;
bool possibleLongPress = false;

// We use this variables inside the interupt in order to inform the code hat the switch was pressed
volatile bool possiblePress = false;
volatile bool shouldStartCounter = false;

// We use a buzzer to make a sound when we reset the segments
const int buzzerPin = 13;
const int buzzerTone = 3500;
const int buzzerDuration = 500;

int segments[numberOfSegments] = {
  pinA, pinB, pinC, pinD, pinE, pinF, pinG, pinDP
};

bool segmentStates[numberOfSegments] = {
  LOW, LOW, LOW, LOW, LOW, LOW, LOW
};

// We use this matrix to navigate the segments with the joystick
int segmentMatrix[numberOfSegments][numberOfDirections] = {
  // U  D  L  R
  { -1, 6, 5, 1 },    // a 0
  { 0, 6, 5, -1 },    // b 1
  { 6, 3, 4, 7 },     // c 2
  { 6, -1, 4, 2 },    // d 3
  { 6, 3, -1, 2 },    // e 4
  { 0, 6, -1, 1 },    // f 5
  { 0, 3, -1, -1 },   // g 6
  { -1, -1, 2, -1 },  // dp 7
};

#define UP 0
#define DOWN 1
#define LEFT 2
#define RIGHT 3

void setup() {
  Serial.begin(9600);
  // Initialize joystick pins
  pinMode(pinSW, INPUT_PULLUP);

  // Initialize all the pins
  for (int i = 0; i < numberOfSegments; i++) {
    pinMode(segments[i], OUTPUT);
  }

  // Initialize the interrupt
  attachInterrupt(digitalPinToInterrupt(pinSW), handleInterrupt, FALLING);
}

void loop() {
  //move() handles the navigation through the segments using the joystick
  move();
  //blink() handles the blinking of the current segment
  blink();
  //shortPress() handles the short press of the switch
  shortPress();
  //longPress() handles the long press of the switch
  longPress();
}

void blink() {
  // If enough time has passed, we change the state of the current segment
  if (millis() - lastBlink > blinkDelay) {
    digitalWrite(segments[currentSegment], blinkState);
    blinkState = !blinkState;
    lastBlink = millis();
  }
}

void move() {
  // We read the values of pin X and Y
  xValue = analogRead(pinX);
  yValue = analogRead(pinY);
  // If the joystick did not move before, we check if it passed any threshold
  // and if the move is valid on the segmentMatrix we turn off or on the segment
  // and change the currentSegment
  if (joyMoved == false) {
    if (xValue < minInputThreshold) {
      if (segmentMatrix[currentSegment][LEFT] != -1) {
        digitalWrite(segments[currentSegment], segmentStates[currentSegment]);
        currentSegment = segmentMatrix[currentSegment][LEFT];
      }
      joyMoved = true;
    } else if (xValue > maxInputThreshold) {
      if (segmentMatrix[currentSegment][RIGHT] != -1) {
        digitalWrite(segments[currentSegment], segmentStates[currentSegment]);
        currentSegment = segmentMatrix[currentSegment][RIGHT];
      }
      joyMoved = true;
    } else if (yValue < minInputThreshold) {
      if (segmentMatrix[currentSegment][UP] != -1) {
        digitalWrite(segments[currentSegment], segmentStates[currentSegment]);
        currentSegment = segmentMatrix[currentSegment][UP];
      }
      joyMoved = true;
    } else if (yValue > maxInputThreshold) {
      if (segmentMatrix[currentSegment][DOWN] != -1) {
        digitalWrite(segments[currentSegment], segmentStates[currentSegment]);
        currentSegment = segmentMatrix[currentSegment][DOWN];
      }
      joyMoved = true;
    }
    // If the values are again in the Threshold we set joyMoved to false
    // so we can move again
  } else if ((xValue >= minIdleThreshold && xValue <= maxIdleThreshold)
             && (yValue >= minIdleThreshold && yValue <= maxIdleThreshold)) {
    joyMoved = false;
  }
}

void shortPress() {
  // We check if possiblePress is true
  if (possiblePress) {
    // We start the debounce counter if needed
    if (shouldStartCounter) {
      lastClick = millis();
      longPressTime = millis();
      shouldStartCounter = false;
    }
    // If the debounce period has passed we the current state of the switch
    if ((millis() - lastClick) > inputDebounceDelay) {
      stateReading = digitalRead(pinSW);
      if (stateReading != swState) {
        // If the button state has changed and it is LOW, we change the state of the segment where the current segment is
        //(not to be confused with blinkState) and set possibleLongPress to true
        if (stateReading == LOW) {
          segmentStates[currentSegment] = !segmentStates[currentSegment];
          possibleLongPress = true;
        }
      }
      possiblePress = false;
    }
  }
}

void longPress() {
  // If long press is true and the restTime passed we check if the switch is still pressed
  if (possibleLongPress) {
    if (millis() - longPressTime > resetTime) {
      stateReading = digitalRead(pinSW);
      if (stateReading != swState) {
        if (stateReading == LOW) {
  // If all the conditions are met we turn all the segments off, we set the currentSegment to DP(decimal point segment)
  // and start the buzzer 
          for (int seg = firstSegment; seg < numberOfSegments; seg++) {
            segmentStates[seg] = LOW;
            digitalWrite(segments[seg], segmentStates[seg]);
          }
          currentSegment = DPsegment;
          tone(buzzerPin, buzzerTone, buzzerDuration);
        }
        possibleLongPress = false;
      }
    }
  }
}

// This is the interrupt function. It is called every time we press the switch
// and it sets the possiblePress and shouldStartCounter to true so we know the switch has been pressed 
void handleInterrupt() {
  possiblePress = true;
  shouldStartCounter = true;
}