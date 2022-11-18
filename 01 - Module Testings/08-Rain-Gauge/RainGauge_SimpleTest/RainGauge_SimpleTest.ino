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
