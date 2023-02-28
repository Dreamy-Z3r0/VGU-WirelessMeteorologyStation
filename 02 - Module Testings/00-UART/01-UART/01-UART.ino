/************************************************************
 ************************************************************
 ***   UART test for STM32F103CBT6 on Arduino framework   ***
 *** ---------------------------------------------------- ***
 ***                                                      ***
 *** The generic Serial name is mapped to U(S)ART 1 of    ***
 *** the STM32 microcontroller: (RX1, TX1) = (PA10, PA9)  ***
 ***                                                      ***
 *** Serial1 works, but is also mapped to U(S)ART 1.      ***
 *** There are no Serial2 and Serial3. In order to use    ***
 *** U(S)ART 2 and U(S)ART 3, it is necessary to declare  ***
 *** new HardwareSerial objects:                          ***
 ***    HardwareSerial Serial2(PA3, PA2);                 ***
 ***    HardwareSerial Serial3(PB11, PB10);               ***
 ************************************************************
 ************************************************************/

#include "Macros_and_Variables.h"

void setup() {
  Serial.begin(9600);
}

void loop() {
  Serial.print("counter = ");
  Serial.println(counter);

  if (10 == counter) dir = -1;
  if (-10 == counter) dir = 1;
  counter += dir;

  delay(delayTime);
}
