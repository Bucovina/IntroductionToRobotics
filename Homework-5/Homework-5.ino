/*
  Pseudo-smart environment monitor and logger 

  Description:
  This assignment focuses on developing a ”Smart Environment Monitor and Logger” using Arduino.
  This system will utilize various sensors to gather environmental data, log this data into EEPROM,
  and provide both visual feedback via an RGB LED and user interaction through a Serial Menu.

  Components:
    • one ultrasonic sensor
    • one ldr sensor
    • one pressure sensor
    • one RGB LED
    • resistors and wires as needed

  Created 21-11-2023
  Coriciuc Tudor 

  https://youtu.be/81OM5VwDpT4
*/

#include <EEPROM.h>
// We use eeprom to save variables to not lose them after a restart

const int trigPin = 6;
const int echoPin = 7;
const int ldrPin = A0;
const int pressurePin = A1;
const int redLedPin = 11;
const int greenLedPin = 10;
const int blueLedPin = 9;

int optionMainMenu = 0;
int optionSubMenu = 0;

// SamplingInterval is saved at byte 0 and 1(int) on eeprom
int samplingInterval;
int samplingIntervalByte = 0;
unsigned long lastRead = 0;
unsigned long lastShown = 0;

// LEDs brightness is between 0-255
const int minLedValue = 0;
const int maxLedValue = 255;

// Min and max Thresholds for sensors
const int minSampling = 1;
const int maxSampling = 10;
const int minUltrasonicThreshold = 1;
const int maxUltrasonicThreshold = 100;
const int minPressureThreshold = 1;
const int maxPressureThreshold = 1023;
const int minLdrThreshold = 1;
const int maxLdrThreshold = 1000;

// Threshold variables and the byte where is saved in eeprom
int pressureThreshold;
int ldrThreshold;
int ldrThresholdByte = 2;
int ultrasonicThreshold;
int ultrasonicThresholdByte = 4;

// How many saves we have for each sensor
const int numberOfSaves = 10;

// Ultrasonic sensor variables
const float speedOfSound = 0.034;
const int numberOfBounces = 2;

// Validate that you want to delete the saves
int sure;

bool shown = false;

int ultrasonicData[numberOfSaves];
int ldrData[numberOfSaves];
int pressureData[numberOfSaves];

#define NRCOLORS 3

int rbgCollors = NRCOLORS;


// One variable for each color of the RGB LED and where are they saved in eeprom
int rValue;
int rValueByte = 6;
int gValue;
int gValueByte = 8;
int bValue;
int bValueByte = 10;

// RGB LED mode auto or manual
bool autoRBG = true;
int autoRBGByte = 12;

#define INITIALVALUE 0
#define MILLIESTOSEC 1000

void setup() {
  // Pin declaration
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(ldrPin, INPUT);
  pinMode(pressurePin, INPUT);

  // We get the saved data from the eeprom 
  EEPROM.get(samplingIntervalByte, samplingInterval);
  EEPROM.get(ultrasonicThresholdByte, ultrasonicThreshold);
  EEPROM.get(ldrThresholdByte, ldrThreshold);
  EEPROM.get(rValueByte, rValue);
  EEPROM.get(gValueByte, gValue);
  EEPROM.get(bValueByte, bValue);
  EEPROM.get(autoRBGByte, autoRBG);

  // We set the baud rate for the serial and print the menu
  Serial.begin(9600);
  printMainMenu();
}


void loop() {
  // This function handles the input for the meniu in serial
  inputHandling();

  // This function handles the LED and Sensors readings  
  sensorsHandling();
}

void inputHandling() {
  // If we select an option in the main menu if possible
  if (optionMainMenu == 0) {
    if (Serial.available() > 0) {
      optionMainMenu = Serial.parseInt();
      selectMainMenu(optionMainMenu);
    }
  } else {
    // If we select an option in the sub menu if possible
    // and call a function for what we need
    if (optionSubMenu == 0) {
      if (Serial.available() > 0) {
        optionSubMenu = Serial.parseInt();
      }
    }
    switch (optionMainMenu) {
      case 1:
        switch (optionSubMenu) {
          case 1:
            sensorSamplingInterval();
            break;
          case 2:
            ultrasonicAlertThreshold();
            break;
          case 3:
            ldrAlertThreshold();
            break;
          case 4:
            pressureAlertThreshold();
            break;
          case 5:
            back();
            break;
          default:
            if (optionSubMenu != 0) {
              // If the option is not available send this message and try again
              Serial.println(F("Invalid option. Try again!"));
            }
            optionSubMenu = 0;
            break;
        }
        break;
      case 2:
        switch (optionSubMenu) {
          case 1:
            resetLoggerData();
            break;
          case 2:
            back();
            break;
          default:
            if (optionSubMenu != 0) {
              Serial.println(F("Invalid option. Try again!"));
            }
            optionSubMenu = 0;
            break;
        }
        break;
      case 3:
        switch (optionSubMenu) {
          case 1:
            currentSensorReadings();
            break;
          case 2:
            currentSensorSettings();
            break;
          case 3:
            displayLoggedData();
            break;
          case 4:
            back();
            break;
          default:
            if (optionSubMenu != 0) {
              Serial.println(F("Invalid option. Try again!"));
            }
            optionSubMenu = 0;
            break;
        }
        break;
      case 4:
        switch (optionSubMenu) {
          case 1:
            manualColorControl();
            break;
          case 2:
            ledToggleAutomaticOnOff();
            break;
          case 3:
            back();
            break;
          default:
            if (optionSubMenu != 0) {
              Serial.println(F("Invalid option. Try again!"));
            }
            optionSubMenu = 0;
            break;
        }
        break;
      case 5:
        back();
        break;
      default:
        break;
    }
  }
}

void printMainMenu() {
  Serial.println(F("Main Menu:"));
  Serial.println(F("Select an option:"));
  Serial.println(F("1. Sensors Settings"));
  Serial.println(F("2. Reset Logger Data"));
  Serial.println(F("3. System Status"));
  Serial.println(F("4. RGB LED Control"));
  Serial.println(F("5. Reprint MainMenu"));
}

void selectMainMenu(int option) {
  /// Print the main menu selected option
  switch (option) {
    case 1:
      Serial.println(F("1. Sensors Settings:"));
      Serial.println(F("  1.1 Sensors Sampling Interval"));
      Serial.println(F("  1.2 Ultrasonic Alert Threshold"));
      Serial.println(F("  1.3 LDR Alert Threshold"));
      Serial.println(F("  1.4 Pressure Alert Threshold"));
        Serial.println(F("  1.5 Back"));
      break;
    case 2:
      Serial.println(F("2. Reset Logger Data:"));
      Serial.println(F("  2.1 Yes"));
      Serial.println(F("  2.2 No"));
      break;
    case 3:
      Serial.println(F("3. System Status:"));
      Serial.println(F("  3.1 Current Sensor Readings"));
      Serial.println(F("  3.2 Current Sensor Settings"));
      Serial.println(F("  3.3 Display Logged Data"));
      Serial.println(F("  3.4 Back"));
      break;
    case 4:
      Serial.println(F("4. RGB LED Control:"));
      Serial.println(F("  4.1 Manual Color Control"));
      Serial.println(F("  4.2 LED: Toggle Automatic ON/OFF"));
      Serial.println(F("  4.3 Back"));
      break;
    case 5:
      break;
    default:
      Serial.println(F("Invalid Input. Try again!"));
      back();
      break;
  }
}

void sensorSamplingInterval() {
  // Choose a sampling rate and save it in the eeprom
  if (shown == false) {
    Serial.println("Choose a number between 1 and 10 for the sampling interval of the sensors:");
    shown = true;
  }
  if (Serial.available() > 0) {
    samplingInterval = Serial.parseInt();
    if (samplingInterval >= minSampling && samplingInterval <= maxSampling) {
      EEPROM.put(samplingIntervalByte, samplingInterval);
      Serial.print(samplingInterval);
      Serial.println(" seconds time for the sampling interval!");
      back();
    } else {
      Serial.println("Invalid Input. Try again!");
    }
  }
}

void ultrasonicAlertThreshold() {
  // Read the serial.print for explanation
  if (shown == false) {
    Serial.println("Choose a number (cm) for the threshold of the ultrasonic sensor:");
    shown = true;
  }
  if (Serial.available() > 0) {
    ultrasonicThreshold = Serial.parseInt();
    if (ultrasonicThreshold >= minUltrasonicThreshold && ultrasonicThreshold <= maxUltrasonicThreshold) {
      // We save the ultrasonic threshold in eeprom
      EEPROM.put(ultrasonicThresholdByte, ultrasonicThreshold);
      Serial.print(ultrasonicThreshold);
      Serial.println(" cm for the ultrasonic sensor threshold!");
      back();
    } else {
      Serial.println("Invalid Input. Try again!");
    }
  }
}

void ldrAlertThreshold() {
  // Read the serial.print for explanation
  if (shown == false) {
    Serial.println("Choose a number for the threshold of the ldr sensor:");
    shown = true;
  }
  if (Serial.available() > 0) {
    ldrThreshold = Serial.parseInt();
    if (ldrThreshold >= minLdrThreshold && ldrThreshold <= maxLdrThreshold) {
      EEPROM.put(ldrThresholdByte, ldrThreshold);
      Serial.print(ldrThreshold);
      Serial.println(" for the ldr sensor threshold!");
      back();
    } else {
      Serial.println("Invalid Input. Try again!");
    }
  }
}

void pressureAlertThreshold() {
  // Read the serial.print for explanation
  if (shown == false) {
    Serial.println("Choose a number for the threshold of the pressure sensor:");
    shown = true;
  }
  if (Serial.available() > 0) {
    pressureThreshold = Serial.parseInt();
    if (pressureThreshold >= minPressureThreshold && pressureThreshold <= maxPressureThreshold) {
      Serial.print(pressureThreshold);
      Serial.println(" for the pressure sensor threshold!");
      back();
    } else {
      Serial.println("Invalid Input. Try again!");
    }
  }
}

void resetLoggerData() {
  // We reset the saved data if we are sure we want this thing to happen
  if (shown == false) {
    Serial.println("Are you sure?");
    Serial.println("1 for YES, 0 for NO");
    shown = true;
  }
  if (Serial.available()) {
    sure = Serial.parseInt();
    if (sure == 1) {
      for (int data = 0; data < numberOfSaves; data++) {
        ultrasonicData[data] = 0;
        ldrData[data] = 0;
        pressureData[data] = 0;
      }
      Serial.println("Data reseted!");
      back();
    } else {
      back();
    }
  }
}

void currentSensorReadings() {
  // We print in the serial the latest data recived every samping rate seconds
  // Press 0 to exit
  if (shown == false) {
    Serial.println("Press 0 to exit!");
    shown = true;
  }
  if (millis() - lastShown >= samplingInterval * MILLIESTOSEC) {
    Serial.print("Ultrasonic sensor: ");
    Serial.println(ultrasonicData[INITIALVALUE]);
    Serial.print("LDR sensor: ");
    Serial.println(ldrData[INITIALVALUE]);
    Serial.print("Pressure sensor: ");
    Serial.println(pressureData[INITIALVALUE]);
    lastShown = millis();
  }
  if (Serial.available() > 0) {
    int quit = Serial.parseInt();
    if (quit == 0) {
      back();
    } else {
      Serial.println("Invalid input. Press 0 to exit!");
    }
  }
}

void currentSensorSettings() {
  // Read the serial.print for explanation
  Serial.println("Current setting for the sensors:");
  Serial.print("Sampling interval: ");
  Serial.println(samplingInterval);
  Serial.print("Ultrasonic sensor threshold: ");
  Serial.println(ultrasonicThreshold);
  Serial.print("LDR sensor threshold: ");
  Serial.println(ldrThreshold);
  Serial.print("Pressure sensor threshold: ");
  Serial.println(pressureThreshold);
  back();
}

void displayLoggedData() {
  // Read the serial.print for explanation
  Serial.println("Saved data for ultrasonic sernsor:");
  for (int data = 0; data < numberOfSaves; data++) {
    Serial.print(data + 1);
    Serial.print(". ");
    Serial.println(ultrasonicData[data]);
  }
  Serial.println("Saved data for ldr sernsor:");
  for (int data = 0; data < numberOfSaves; data++) {
    Serial.print(data + 1);
    Serial.print(". ");
    Serial.println(ldrData[data]);
  }
  Serial.println("Saved data for pressure sernsor:");
  for (int data = 0; data < numberOfSaves; data++) {
    Serial.print(data + 1);
    Serial.print(". ");
    Serial.println(pressureData[data]);
  }
  back();
}

void manualColorControl() {
  // We can choose a custom color for the LED when it is in manual mode
  if (shown == false) {
    switch (rbgCollors) {
      case 3:
        Serial.println("Choose intensity for Red:");
        break;
      case 2:
        Serial.println("Choose intensity for Green:");
        break;
      case 1:
        Serial.println("Choose intensity for Blue:");
        break;
      default:
        break;
    }
    shown = true;
  }

  if (Serial.available() > 0) {
    int color = Serial.parseInt();
    if (color >= minLedValue && color <= maxLedValue) {
      switch (rbgCollors) {
        case 3:
          rValue = color;
          EEPROM.put(rValueByte, rValue);
          Serial.print("RED intensity is: ");
          Serial.println(rValue);
          rbgCollors--;
          shown = false;
          break;
        case 2:
          gValue = color;
          EEPROM.put(gValueByte, gValue);
          Serial.print("GREEN intensity is: ");
          Serial.println(gValue);
          rbgCollors--;
          shown = false;
          break;
        case 1:
          bValue = color;
          EEPROM.put(bValueByte, bValue);
          Serial.print("BLUE intensity is: ");
          Serial.println(bValue);
          rbgCollors--;
          shown = false;
          break;
        default:
          break;
      }
      if (rbgCollors == 0) {
        rbgCollors = NRCOLORS;
        back();
      }
    } else {
      Serial.println("Invalid Input. Try again!");
    }
  }
}

void ledToggleAutomaticOnOff() {
  // Switch the LED from manual to automatic
  if (autoRBG == true) {
    Serial.println("LED Automatic: ON -> OFF");
    autoRBG = false;
  } else {
    Serial.println("LED Automatic: OFF -> ON");
    autoRBG = true;
  }
  EEPROM.put(autoRBGByte, autoRBG);
  back();
}

void back() {
  // We use this function to go back to the main menu
  // We reset the menus options and print the main menu back
  // We use this function a lot
  optionMainMenu = 0;
  optionSubMenu = 0;
  shown = 0;
  printMainMenu();
}

int ultrasonicRead() {
  // We read the data from the ultrasonic sensor
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  int duration = pulseIn(echoPin, HIGH);
  int distance = duration * speedOfSound / numberOfBounces;
  return distance;
}

int ldrRead() {
  // We read the data from the ldr sensor
  return analogRead(ldrPin);
}

int pressureRead(){
  // We read the data from the pressure sensor
  return analogRead(pressurePin);
}

void saveData(int ultrasonic, int ldr,int pressure) {
  /// We save the last reading and overwrite the oldest saved data
  for (int save = numberOfSaves - 1; save > INITIALVALUE; save--) {
    ultrasonicData[save] = ultrasonicData[save - 1];
    ldrData[save] = ldrData[save - 1];
    pressureData[save]=pressureData[save-1];
  }
  ultrasonicData[INITIALVALUE] = ultrasonic;
  ldrData[INITIALVALUE] = ldr;
  pressureData[INITIALVALUE]=pressure;
}

void sensorsHandling() {
  // We check the sensors every sampling rate seconds
  if (millis() - lastRead >= samplingInterval * MILLIESTOSEC) {
    // We read the data from each sensor 
    int ultrasonicDataReading = ultrasonicRead();
    int ldrDataReading = ldrRead();
    int pressureDataReading =pressureRead();
    // If the led in automatic mode we make the led green if all the sensors are in the thresholds else we make it red
    // and print an alert message for the individual sensor
    if (autoRBG == true) {
      if (ultrasonicDataReading < ultrasonicThreshold || ldrDataReading < ldrThreshold || pressureDataReading>pressureThreshold) {
        analogWrite(redLedPin, maxLedValue);
        analogWrite(greenLedPin, minLedValue);
        analogWrite(blueLedPin, minLedValue);
      } else {
        analogWrite(redLedPin, minLedValue);
        analogWrite(greenLedPin, maxLedValue);
        analogWrite(blueLedPin, minLedValue);
      }
      if (ultrasonicDataReading < ultrasonicThreshold) {
        Serial.println("ALERT!!!: Gremlin Incoming!");
      }
      if (ldrDataReading < ldrThreshold) {
        Serial.println("ALERT!!!: Darkness Calling!");
      }
      if(pressureDataReading>pressureThreshold){
        Serial.println("ALERT!!!: Elephant Detected!");
      }
    } else {
      // If the sensor is in manual mode we use the custom color we choose 
      analogWrite(redLedPin, rValue);
      analogWrite(greenLedPin, gValue);
      analogWrite(blueLedPin, bValue);
    }
    // We save the data and update the lastRead
    saveData(ultrasonicDataReading, ldrDataReading, pressureDataReading);
    lastRead = millis();
  }
}