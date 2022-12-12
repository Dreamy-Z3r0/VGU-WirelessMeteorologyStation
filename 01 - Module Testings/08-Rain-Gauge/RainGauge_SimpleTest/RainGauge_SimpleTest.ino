/******************************************************************************
 ******************************************************************************
 ***   Rain gauge interfacing test for STM32F103CBT6 on Arduino framework   ***
 *** ---------------------------------------------------------------------- ***
 ***                                                                        ***
 *** The rain gauge in use is a self-emptying tipping bucket that gives of  ***
 *** a pulse for every 0.2794 mm of rainfall.                               ***
 ***                                                                        ***
 *** This scope aims at testing interfacing the rain gauge and the micro-   ***
 *** controller. Moreover, it is used to help determine the necessary low-  ***
 *** pass filter which denoises signals from the sensor.                    ***
 ***                                                                        ***
 *** Setup:                                                                 ***
 ***   (1) Tipping bucket --> Microcontroller                               ***
 ***                      |-> Logic analyzer                                ***
 ***                      |-> Oscilloscope                                  ***
 ***                                                                        ***
 ***   (2) Tipping bucket --> LPF --> Microcontroller                       ***
 ***                              |-> Logic analyzer                        ***
 ***                              |-> Oscilloscope                          ***
 ***                                                                        ***
 *** Note: Logic analyzer is used to validate the resolution of the rain    ***
 ***       gauge (1 pulse for 0.2794mm of precipitation).                   ***
 ******************************************************************************
 ******************************************************************************/

#define rainGaugeInput PB3
#define rainfallPerTip 0.2794

unsigned long rainGaugeCounter = 0;
double precipitationOutput;

void setup() {
  Serial.begin(9600);
  
  pinMode(rainGaugeInput, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(rainGaugeInput), RainGauge_Tipping_Callback, FALLING);
}

void loop() {
  precipitationOutput = rainGaugeCounter * rainfallPerTip;

  if (Serial.available()) {
    String serialInput = Serial.readStringUntil('\n');

    if (serialInput.equals("data")) {
      Serial.print("Current count: ");
      Serial.print(rainGaugeCounter);
      Serial.println(" tips");
      
      Serial.print("Rainfall amount: ");
      Serial.print(precipitationOutput);
      Serial.println(" mm\n");
    } else if (serialInput.equals("reset")) {
      rainGaugeCounter = 0;
      precipitationOutput = 0;
      
      Serial.println("Rain gauge count reset!\n");
    }
  }
}

void RainGauge_Tipping_Callback() {
  rainGaugeCounter += 1;
}
