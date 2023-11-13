/*
  Stopwatch Timer

  Description:
  This assignment focuses on using the 4 digit 7 segment display and 3 buttons,
  we implement a stopwatch timer that counts in 10ths of a secondand and 
  has a save lap functionality (similar to most basic stopwatch functionson most phones).

  Components:
  • one 4 7-segment display
  • three buttons
  • one shift register
  • resistors and wires as needed

  Created 13-11-2023
  Coriciuc Tudor 

  https://youtu.be/EmrBHu0CKMo
*/


// Define connections to the shift register
const int latchPin = 11;  // Connects to STCP (latch pin) on the shift register
const int clockPin = 10;  // Connects to SHCP (clock pin) on the shift register
const int dataPin = 12;   // Connects to DS (data pin) on the shift register

// Define connections to the digit control pins for a 4-digit display
const int segD1 = 4;
const int segD2 = 5;
const int segD3 = 6;
const int segD4 = 7;

//Buttons declaration
const int startStopPin = 2;
const int lapPin = 3;
const int restartPin = 8;

// Store the digits in an array for easy access
int displayDigits[] = { segD1, segD2, segD3, segD4 };
// Number of digits in the display
const int displayCount = 4;
// Define the number of unique encodings (0-9)
const int encodingsNumber = 10;

// Logic variables
bool start = false;
bool inLap = false;
bool restarted = true;

// We use interrupt for start/stop button and lap button
volatile bool possiblePressStartStop = false;
volatile bool shouldStartCounterStartStop = false;

volatile bool possiblePressLap = false;
volatile bool shouldStartCounterLap = false;

// We use debouncing on the each button
bool stateReadingStartStop;
bool stateReadingRestart;
bool stateReadingLap;
const int DebounceDelay = 100;
unsigned long lastStartStop = 0;
unsigned long lastLap = 0;
unsigned long lastReset = 0;

// We can store maxium four laps at a time
const int numberOfLaps = 4;
int savedLaps[4] = { 0, 0, 0, 0 };
int currentLapShown = 0;

int restartState;
int lastRestartState = HIGH;

// Define byte encodings for the hexadecimal characters 0-F
byte byteEncodings[encodingsNumber] = {
  //A B C D E F G DP
  B11111100,  // 0
  B01100000,  // 1
  B11011010,  // 2
  B11110010,  // 3
  B01100110,  // 4
  B10110110,  // 5
  B10111110,  // 6
  B11100000,  // 7
  B11111110,  // 8
  B11110110,  // 9
};
#define DECIMALPOINT B00000001
#define CLEARDISPLAY B00000000
#define TIMESTART 0
#define INITIALLAP 0

// Variables for controlling the display update timing
unsigned long lastIncrement = 0;
unsigned long delayCount = 100;  // Delay between updates (milliseconds)
unsigned long number = 0;        // The number being displayed

void setup() {
  // Initialize the pins connected to the shift register as outputs
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, OUTPUT);
  // Initialize buttons
  pinMode(startStopPin, INPUT_PULLUP);
  pinMode(restartPin, INPUT_PULLUP);
  pinMode(lapPin, INPUT_PULLUP);
  // Initialize digit control pins and set them to LOW (off)
  for (int i = 0; i < displayCount; i++) {
    pinMode(displayDigits[i], OUTPUT);
    digitalWrite(displayDigits[i], LOW);
  }
  // Begin serial communication for debugging purposes
  Serial.begin(9600);
  // Initialize interrupts
  attachInterrupt(digitalPinToInterrupt(startStopPin), handleInterruptStartStop, FALLING);
  attachInterrupt(digitalPinToInterrupt(lapPin), handleInterruptSaveLaps, FALLING);
}

void loop() {
  // We use functions to check if the buttons are pressed
  startStopPress();
  saveLapPress();
  restartPress();

  //
  if (start && inLap == false) {
    if (millis() - lastIncrement > delayCount) {
      // DONE: Increment the number and reset 'lastIncrement'
      number++;
      // DONE: Ensure that 'number' wraps around after reaching 9999 to start over from 0
      number %= 10000;  // Wrap around after 9999
      lastIncrement = millis();
    }
    // DONE: Display the incremented number on the 7-segment display using multiplexing
  }
  writeNumber(number);
}

void writeReg(int digit) {
  // Prepare to shift data by setting the latch pin low
  digitalWrite(latchPin, LOW);
  // Shift out the byte representing the current digit to the shift register
  shiftOut(dataPin, clockPin, MSBFIRST, digit);
  // Latch the data onto the output pins by setting the latch pin high
  digitalWrite(latchPin, HIGH);
}

void activateDisplay(int displayNumber) {
  // Turn off all digit control pins to avoid ghosting
  for (int i = 0; i < displayCount; i++) {
    digitalWrite(displayDigits[i], HIGH);
  }
  // Turn on the current digit control pin
  digitalWrite(displayDigits[displayNumber], LOW);
}

void writeNumber(int number) {
  // We initialize necessary variables for tracking the current number and digit position
  int currentNumber = number;
  int displayDigit = 3;  // Start with the least significant digit
  int lastDigit = 0;
  // We loop through each digit of the current number
  while (displayDigit >= 0) {
    // We extract the last digit of the current number
    lastDigit = currentNumber % encodingsNumber;
    // we activate the current digit on the display
    activateDisplay(displayDigit);
    // We output the byte encoding for the last digit to the display
    // The second most significant digit will always have the decimal digit turned on
    displayDigit == 2 ? writeReg(byteEncodings[lastDigit] + DECIMALPOINT) : writeReg(byteEncodings[lastDigit]);
    // We go to the next digit
    displayDigit--;
    // Update 'currentNumber' by removing the last digit
    currentNumber /= encodingsNumber;
    // Clear the display to prevent ghosting between digit activations
    writeReg(CLEARDISPLAY);
  }
}

void startStopPress() {
  // If we pressed the start/stop button we start the counter
  if (possiblePressStartStop) {
    if (shouldStartCounterStartStop) {
      lastStartStop = millis();
      shouldStartCounterStartStop = false;
    }
    // If enough time has passed between the last time we pressed the button and the button is pressed
    // we check if we are not in saved laps view mode(inLap is false)
    // and change start value(if start is true we are counting else we are not)
    // we also change restarted value to false
    if ((millis() - lastStartStop) > DebounceDelay) {
      stateReadingStartStop = digitalRead(startStopPin);
      if (stateReadingStartStop == LOW) {
        if (inLap == false) {
          start = !start;
          restarted = false;
        }
      }
      possiblePressStartStop = false;
    }
  }
}

void saveLapPress() {
  // If we pressed the lap button we start the counter
  if (possiblePressLap) {
    if (shouldStartCounterLap) {
      lastLap = millis();
      shouldStartCounterLap = false;
    }
    if ((millis() - lastLap) > DebounceDelay) {
      stateReadingLap = digitalRead(lapPin);
      if (stateReadingLap == LOW) {
        // If the timer is counting we can save a lap(maximum four)
        if (start == true) {
          for (int lap = numberOfLaps - 1; lap > INITIALLAP; lap--) {
            savedLaps[lap] = savedLaps[lap - 1];
          }
          savedLaps[INITIALLAP] = number;
        }
        // If counter is not working and we restarted the timer
        // we can enter the saved laps view mode(inLap is true)
        else if (start == false && restarted) {
          inLap = true;
          // If we press the lap button multiple times while in this mode
          // we cycle through the saved laps
          number = savedLaps[(currentLapShown++)];
          // If we reach the fifth lap we go back to the initial saved lap
          if (currentLapShown == numberOfLaps) {
            currentLapShown = INITIALLAP;
          }
        }
      }
      possiblePressLap = false;
    }
  }
}

void restartPress() {
  // The restart function works only if the counter in not on
  // The restart button doesn't have an interrupt
  if (start == false) {
    // We read the current state of the restart button and
    // if it is different from the last state we start the timer
    stateReadingRestart = digitalRead(restartPin);

    if (stateReadingRestart != lastRestartState) {
      lastReset = millis();
    }

    // If enough time passed we check if the current state and
    // the state of the button match
    if (millis() - lastReset > DebounceDelay) {
      if (stateReadingRestart != restartState) {
        restartState = stateReadingRestart;
        if (restartState == LOW) {
          // If we are in the saved laps view mode and we press the restart button
          // we set all the lap times to 000.0 and we exit the saved laps view mode
          if (inLap) {
            for (int lap = INITIALLAP; lap < numberOfLaps; lap++) {
              savedLaps[lap] = TIMESTART;
            }
            inLap = false;
          }
          // We set the counter to 000.0, the current lap to 0 and
          // mark that we restarted
          number = TIMESTART;
          currentLapShown = INITIALLAP;
          restarted = true;
        }
      }
    }
    lastRestartState = stateReadingRestart;
  }
}

// Interrupts
void handleInterruptStartStop() {
  possiblePressStartStop = true;
  shouldStartCounterStartStop = true;
}

void handleInterruptSaveLaps() {
  possiblePressLap = true;
  shouldStartCounterLap = true;
}