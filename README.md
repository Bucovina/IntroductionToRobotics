# Introduction to Robotics (2023-2024)

This repository showcases my journey through the Introduction to Robotics laboratory, taken in the 3rd year at the Faculty of Mathematics and Computer Science, University of Bucharest. Each step I took is represented by the homework I uploaded here, including requirements, implementation details, code, and even photos and videos.

## Table of contents
1. [Homework-1](#Homework-1)
2. [Homework-2](#Homework-2)
3. [Homework-3](#Homework-3)
4. [Homework-4](#homework-4)


## Homework-1 
Description:

This first mission consists of the implementation of a circuit made up of an RGB LED and three potentiometers. We use the three potentiometers and the Arduino board to control the light intensity of each channel of the RGB LED. The RGB LED has 4 pins, 3 for colors and one for ground. The required voltages for the red pin is 2.0 V, for the green one is 3.2 V and for the blue one is 3.1 V. We also used three resistors, two of 330 Ohms and one of 220 Ohms. A potentiometer has 3 pins, 2 of them are called fixed and are for power and ground, respectively. The other pin is called variable and we use it to change the luminous intensity of the LED.

Used components:

  • one RGB LED
  
  • three Potentiometers
  
  • resistors and wires as needed

Video:

https://youtu.be/4cim3AliW1Y

Image:

![Homework 1 - LED](https://github.com/Bucovina/IntroductionToRobotics/assets/103101131/aa76264f-ad25-488d-8ab5-d9fdcbd73bad)

## Homework-2
Description:

The next stage of the journey consists of simulating a control system of a three-floor elevator. We use three buttons and three green LEDs, one of each for each floor, and a red LED to indicate the operating status of the elevator (if the elevator is not moving, the LED is constantly lit, and if it is moving, the LED blinks) and a buzzer that signals with a sound if the elevator closes, it travels or has reached its destination. To ensure the optimal operation of the buttons, we use a pull-up resistor (more precisely, the internal one that comes with the Arduino board), this way we eliminate the floating values ​​that may appear. We also implemented a debounce system in the code to avoid fictitious button presses due to mechanical and physical issues.

Used components:

  • three green LEDs
  
  • one red LED
  
  • three push-buttons
  
  • one buzzer
  
  • resistors and wires as needed

Video:

https://youtu.be/1rviJSz7--0

Image:

![Homework-2](https://github.com/Bucovina/IntroductionToRobotics/assets/103101131/17556593-1c23-4703-b2fb-9b95cbfefa66)

## Homework-3
Description:

The journey continues with the implementation of a 7 segment display drawing. We use a 7-segment display as a canvas, and with the help of a joystick, we move through the segments and turn them on and off to draw what we want. A joystick is actually two potentiometers and a button or a switch. Reading the values ​​we receive, we know in which direction to go(up, down, left, right). We have implemented some thresholds in the code so that there are no faulty movements. When we press the joystick, it calls an interrupt function that informs us that the button has been pressed and changes the state of the current segment. If we keep the button pressed for a longer time, all the segments turn off.

Used components:

  • one 7-segment display
  
  • one joystick
  
  • one buzzer
  
  • resistors and wires as needed

Video:

https://youtu.be/L5BbXVPg4Ss

Image:

![Homework-3](https://github.com/Bucovina/IntroductionToRobotics/assets/103101131/58a71837-8a78-4678-91ee-36705080cbf7)

## Homework-4
Description:

The following challenge consists of the implementation of a stopwatch. We use a 4 7-segment display and three buttons. Since the 4 7-segment display has 12 pins, we use a shift register to send a serial signal as input and to obtain a parallel signal as output in this way saving some pins of the Arduino. The first button starts and pauses the timer, the second one resets the timer if in pause mode and resets the saved laps, if in lap viewing mode, and the last one saves the laps if the stopwatch is in counting mode or cycles through the last saved laps (up to 4 laps) if in lap viewing mode.

Used components:

  • one 7-segment display

  • three buttons

  • one shift register

  • resistors and wires as needed

Video:

https://youtu.be/EmrBHu0CKMo

Image:

