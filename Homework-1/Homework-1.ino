/*
  RGB

  Description:
  This assignment focuses on controlling each channel (Red, Green, and Blue)
  of  an  RGB  LED  using  individual  potentiometers.

  Components:
  • one RGB LED
  • three Potentiometers
  • resistors and wires as needed

  Created 23-10-2023
  Coriciuc Tudor 
  Modified 06-11-2023
  Coriciuc Tudor 
  
  https://youtu.be/4cim3AliW1Y
*/

/// we name the analog input A2 as varIntPinRed
/// we connect a potentiometer to the A pins (Analog pins)
/// to change the luminous intensity of the LED
const int varIntPinRed = A2;
/// we name the digital PWM Pin 11 as ledPinRed
/// each ledPin will be connected to one color(red, green, blue)
/// of the LED and will sent the intensity(from 0 to 255)
const int ledPinRed = 11;

const int varIntPinGreen = A1;
const int ledPinGreen = 10;

const int varIntPinBlue = A0;
const int ledPinBlue = 9;

/// LEDs brightness is between 0-255
const int minLedValue = 0;
const int maxLedValue = 255;
/// potentiometer have values between 0-1023
const int minIntValue = 0;
const int maxIntValue = 1023;
/// we are going to read 10 times the values varIntPin in order to make an average
const int nrRepForAvg = 10;
const int firstRep = 1;
/// we are going to 5 milisecond between each read
const int minDelay = 5;

int intRedValue = 0;
int intGreenValue = 0;
int intBlueValue = 0;

void setup() {
  /// initialize the serial communication at a baud rate of 9600
  Serial.begin(9600);
  /// set the ledPins as OUTPUT pins
  pinMode(ledPinGreen, OUTPUT);
  pinMode(ledPinRed, OUTPUT);
  pinMode(ledPinBlue, OUTPUT);
}

void loop() {
  /// we read the intensity from the potentiometer and add it in intRedValue
  /// in order to use the average value
  averageValue();

  ///we calculate the average value
  intRedValue /= nrRepForAvg;
  /// we map the values knowing intRedValue
  int mapedRedValue = map(intRedValue, minIntValue, maxIntValue, minLedValue, maxLedValue);
  /// we set the intensity of the red led to the mapedRedValue through ledPinRed
  analogWrite(ledPinRed, mapedRedValue);

  /// we repeat this for the other colors
  intGreenValue /= nrRepForAvg;
  int mapedGreenValue = map(intGreenValue, minIntValue, maxIntValue, minLedValue, maxLedValue);
  analogWrite(ledPinGreen, mapedGreenValue);

  intBlueValue /= nrRepForAvg;
  int mapedBlueValue = map(intBlueValue, minIntValue, maxIntValue, minLedValue, maxLedValue);
  analogWrite(ledPinBlue, mapedBlueValue);
}

void averageValue() {
  for (int i = firstRep; i <= nrRepForAvg; i++) {
    intRedValue += analogRead(varIntPinRed);
    intGreenValue += analogRead(varIntPinGreen);
    intBlueValue += analogRead(varIntPinBlue);
    delay(minDelay);
  }
}