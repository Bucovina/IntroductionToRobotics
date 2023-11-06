/*
  Elevator simulator wannabe

  Description:
  This assignment involves simulating a 3-floor elevator control system
  using LEDs, buttons, and a buzzer with Arduino.

  Components:
  • four LED
  • three Buttons
  • one Buzzer
  • resistors and wires as needed

  Created 30-10-2023
  Coriciuc Tudor
  Modified 06-11-2023
  Coriciuc Tudor 

  https://youtu.be/1rviJSz7--0
*/

//we use an array to store the pins for each LED
//and also the states for the LEDs
const int ledPins[3] = { 11, 10, 9 };
bool ledStates[3] = { HIGH, LOW, LOW };

//we use an other array to store the pins for the buttons and their states
const int buttonPins[3] = { 4, 3, 2 };
bool buttonStates[3];

//we are using two arrays, one for storing the last state of a button
//and checking if the state of a LED changed
//and the other for reading the current state of the LEDs
bool lastButtonStates[3] = { HIGH, HIGH, HIGH };
bool stateReadings[3];

//the operational LED is used to indicate if the elevator is moving or not
//the LED will be on when the elevator is stationary
//and blinking the it is moving
const int operationalLedPin = 8;
bool operationalLedState = HIGH;
//the operational LED blinks every 0.5 seconds
const unsigned int blinkDelay = 500;
unsigned long lastBlink = 0;

//we are debounces for each button to remove the noise
unsigned long lastDebounces[3] = { 0, 0, 0 };
const unsigned int debounceDelay = 50;

//in order to simulate the travel between each level
//there is a 2s delay between one LED turning off and
//the other one turning on
//we are using millies
unsigned long currentLevelChange = 0;
const unsigned int levelChangeDelay = 2000;

//we are simulating a 3-floor elevator
const int numberOfLevels = 3;

bool movingElevator = false;

//there are 3 different sounds
//one for when the door is closing
const int closingDoorsTone = 2000;
const int closingDoorsDuration = 1000;

//one for when the elevator is moving
const int movingTone = 3000;
const int movingDuration = 100;

//and one for when it reaches its destination
const int destinationTone = 5000;
const int destinationDuration = 500;

//current level is where we are right now
//and destination level is where we want to go
int currentLevel = 0;
int destinationLevel = 0;

const int buzzerPin = 7;

const int startingFloor = 0;

void setup() {
  //we set the ledPins to output, the led states
  //and the buttons to input_pullup in order to remove the floating state
  //the button will become active LOW
  for (int level = startingFloor; level < numberOfLevels; level++) {
    pinMode(ledPins[level], OUTPUT);
    digitalWrite(ledPins[level], ledStates[level]);
    pinMode(buttonPins[level], INPUT_PULLUP);
  }

  //we set the operational LED pin to output and its state
  pinMode(operationalLedPin, OUTPUT);
  digitalWrite(operationalLedPin, operationalLedState);
}

void loop() {
  //we read the current state of the button
  for (int level = startingFloor; level < numberOfLevels; level++) {
    stateReadings[level] = digitalRead(buttonPins[level]);
  }

  //if the current state(the reading state) is different
  //from the last state of the button we are updating the lastDebounce time
  for (int level = startingFloor; level < numberOfLevels; level++) {
    if (stateReadings[level] != lastButtonStates[level])
      lastDebounces[level] = millis();
  }

  //if the time between the lastDebounce and the current time
  //is bigger than the debounceDelay we check if the current state
  //and the buttonState are the same
  //if not we update the button state to the current state
  for (int level = startingFloor; level < numberOfLevels; level++) {
    if ((millis() - lastDebounces[level]) > debounceDelay) {
      if (stateReadings[level] != buttonStates[level]) {
        buttonStates[level] = stateReadings[level];
        //if the button state is LOW, the button has been pressed
        //the button is active LOW(because we are using a pullup rezistor)
        //we update the destination level and the currentLevelChange
        //and play the door closing tone
        if (buttonStates[level] == LOW) {
          if (currentLevel == destinationLevel) {
            destinationLevel = level;
            currentLevelChange = millis();
            tone(buzzerPin, closingDoorsTone, closingDoorsDuration);
          }
        }
      }
    }
    //the last button state is updated to current state(stateReadings)
    lastButtonStates[level] = stateReadings[level];
  }

  //if the destiantion level is different from the current level
  //the elevator is moving
  if (destinationLevel != currentLevel) {
    movingElevator = true;

    //we use two if statements to check if the elevator is going up or down
    if (destinationLevel > currentLevel) {
      //if the LED on the current level is on and enough time has passed
      //between the last level change, we turn the LED off and update
      //the currentLevelChange
      if (ledStates[currentLevel] == HIGH && (millis() - currentLevelChange) > levelChangeDelay) {
        ledStates[currentLevel] = LOW;
        digitalWrite(ledPins[currentLevel], ledStates[currentLevel]);
        currentLevelChange = millis();
      }
      //if the LED on the current level is off and enough time has passed
      //between the last level change, we change the currentLevel to the upper one
      //we turn the LED on and update the currentLevelChange
      if (ledStates[currentLevel] == LOW && (millis() - currentLevelChange) > levelChangeDelay) {
        currentLevel++;
        ledStates[currentLevel] = HIGH;
        digitalWrite(ledPins[currentLevel], ledStates[currentLevel]);
        currentLevelChange = millis();
      }
    }
    //same process but he elevator is going down
    //currentLevel is decreasing
    else if (destinationLevel < currentLevel) {
      if (ledStates[currentLevel] == HIGH && (millis() - currentLevelChange) > levelChangeDelay) {
        ledStates[currentLevel] = LOW;
        digitalWrite(ledPins[currentLevel], ledStates[currentLevel]);
        currentLevelChange = millis();
      }
      if (ledStates[currentLevel] == LOW && (millis() - currentLevelChange) > levelChangeDelay) {
        currentLevel--;
        ledStates[currentLevel] = HIGH;
        digitalWrite(ledPins[currentLevel], ledStates[currentLevel]);
        currentLevelChange = millis();
      }
    }
    //if the destination level is equal to current level
    //we arrived and we play the destination tone
    if (destinationLevel == currentLevel) {
      tone(buzzerPin, destinationTone, destinationDuration);
    }
  } else {
    //if the destination level is equal to current level
    //the elevator is not moving
    movingElevator = false;
  }

  //the elevator is not moving the operational LED is turned on
  if (movingElevator == false) {
    operationalLedState = HIGH;
    digitalWrite(operationalLedPin, operationalLedState);
  } else {
    //if the time between the currentLevelChange and the current time
    //is bigger than the closingDoorsDuration we know that the door closing tone
    //is over and we can play the moving tone
    //also the buzz will not be permanenty during the movement,
    //but is intermittent
    if ((millis() - currentLevelChange) > closingDoorsDuration) {
      tone(buzzerPin, movingTone, movingDuration);
    }
    //if the LED on the current level is on and enough time has passed
    //between the last blink, we turn the LED off and update
    //the lastBlink
    if (operationalLedState == HIGH && (millis() - lastBlink) > blinkDelay) {
      operationalLedState = LOW;
      digitalWrite(operationalLedPin, operationalLedState);
      lastBlink = millis();
    }
    //if the LED on the current level is off and enough time has passed
    //between the last blink, we turn the LED on and update
    //the lastBlink
    if (operationalLedState == LOW && (millis() - lastBlink) > blinkDelay) {
      operationalLedState = HIGH;
      digitalWrite(operationalLedPin, operationalLedState);
      lastBlink = millis();
    }
  }
}