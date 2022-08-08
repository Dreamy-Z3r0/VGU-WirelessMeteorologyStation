#define CALX_TEMP 25
#define V25       1430
#define AVG_SLOPE 4300
#define VREFINT   1200

#define ADC_input PA7

unsigned long NumberOfPoints = 100;
bool ADC_12bit = true;

void setup() {
  Serial.begin(9600);
}

void loop() {
  if (Serial.available()) {
    String input = Serial.readStringUntil('\n');

    if (input.equals("start")) {
      ADC_12bit = false;
    }
    else {
      uint8_t separator = input.indexOf(':');
      if (input.substring(0, separator).equals("start")) {
        ADC_12bit = false;
        NumberOfPoints = input.substring(separator+1).toInt();
      }
    }
  }
  else {
    ADC_12bit = true;
  }

  if (!ADC_12bit) {
    Serial.print("Analog read time: (");
    Serial.print(NumberOfPoints);
    Serial.println(" points)");
    
    do {
      uint16_t ADC_value[NumberOfPoints];
      unsigned long t1, t2;
      double T_mean = 0, f = 0;
      uint16_t ADC_RANGE = (ADC_12bit ? 4096 : 1024);

      if (ADC_12bit) {
        analogReadResolution(12);
        Serial.println("  12-bit resolution: ");
      }
      else {
        analogReadResolution(10);
        Serial.println("  10-bit resolution: ");
      }

      Serial.print("    AVref = ");
      Serial.print((VREFINT * ADC_RANGE / analogRead(AVREF)));
      Serial.print(" mV\n    ");

      t1 = micros();
      for (int i = NumberOfPoints-1; i >= 0; i -= 1) {
        ADC_value[i] = analogRead(ADC_input);
      }
      t2 = micros();

      T_mean = ((t2 - t1) / (1.0 * NumberOfPoints));

      f = 1E6 / T_mean;
      
      Serial.print(T_mean, 4);
      Serial.print(" us <=> f = ");

      if (f >= 1E6) {
        Serial.print(f/1E6, 4);
        Serial.println(" MHz");
      }
      else if (f >= 1E3) {
        Serial.print(f/1E3, 4);
        Serial.println(" kHz");
      }
      else {
        Serial.print(f, 4);
        Serial.println(" Hz");
      }

      ADC_12bit = !ADC_12bit;
    } while (ADC_12bit);

    Serial.println();
  }
}
