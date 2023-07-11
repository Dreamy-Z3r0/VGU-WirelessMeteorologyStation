/******************************************************************************
 ******************************************************************************
 ***                      Rain gauge resolution test                        ***
 *** ---------------------------------------------------------------------- ***
 ***                                                                        ***
 *** The rain gauge in use is a self-emptying tipping bucket that gives of  ***
 *** a pulse for every 0.2794 mm of rainfall, according to the datasheets.  ***
 ***                                                                        ***
 *** This scope aims at testing the given resolution of the rain gauge. Any ***
 *** microcontroller could be used for this test, not just the targeted     ***
 *** STM32F103CBT6 of the project.                                          ***
 ***                                                                        ***
 ******************************************************************************
 ******************************************************************************/

#define rainGaugeInput PA1

unsigned long rainGaugeCounter = 0;

void setup() {
  Serial.begin(9600);
  pinMode(rainGaugeInput, INPUT_PULLUP);

  Serial.println("Rain gauge resolution test.");
}

void loop() {
  if (Serial.available()) {
    String serialInput = Serial.readStringUntil('\n');

    // Start a new test cycle
    if (serialInput.equals("begin")) {
      rainGaugeCounter = 0;
      Serial.println("\nBegin test cycle...");

      // Enable inputs from the rain gauge
      attachInterrupt(digitalPinToInterrupt(rainGaugeInput), RainGauge_Tipping_Callback, FALLING);

      // Wait for the stop command
      bool stopCommand = false;
      do {
        if (Serial.available()) {
          serialInput = Serial.readStringUntil('\n');
          if (serialInput.equals("stop")) {
            stopCommand = true;
          }
        }
      } while (!stopCommand);

      // Disable inputs from the rain gauge
      detachInterrupt(digitalPinToInterrupt(rainGaugeInput));

      // Input the amount of water used in the current test cycle
      Serial.print("Amount of water to buckets: ");
      while (!Serial.available());
      serialInput = Serial.readStringUntil('\n');
      float waterAmount = serialInput.toFloat();
      Serial.print(waterAmount);
      Serial.println(" ml");

      // Calculate the rain gauge's resolution based on the input water
      float bucketResolution;
      if (0 < rainGaugeCounter) {
        bucketResolution = waterAmount / rainGaugeCounter;
      } else {
        bucketResolution = 0;
      }

      // Print the test result of the current test cycle
      if (0 == bucketResolution) {
        Serial.println("Error occurred.");
      } else {
        Serial.print("Count: ");
        Serial.println(rainGaugeCounter);
        Serial.print("Resolution of the rain gauge: ");
        Serial.print(bucketResolution);
        Serial.println(" ml per tip");
      }

      Serial.println("End of test cycle.");
    }
  }
}

void RainGauge_Tipping_Callback() {
  rainGaugeCounter += 1;
}
