#define V_ref 3.3           // ADC reference voltage
#define ADC_resolution 12   // STM32F103 has 12-bit ADCs
float   ADC_stepValue;      // Step value = V_ref / (2^ADC_resolution - 1)

#define Vcc 3.3
#define R 3.3E3
#define input_WindVane PB1

float R_in_given[] = {33.1, 6.57, 8.19, 0.89, 1, 0.69, 2.19, 1.4, 3.88, 3.12, 15.98, 14.1, 119.6, 42.07, 64.9, 21.92};

void setup() {
  Serial.begin(9600);
  analogReadResolution(ADC_resolution);   // Overwrite the default 10-bit resolution
  ADC_stepValue = V_ref / (pow(2,ADC_resolution)-1);
}

void loop() {
  float Vin, windDir;
  readRawInstance(&Vin, input_WindVane);
  WindDirectionInstance(&Vin, &windDir);

  Serial.print("Wind direction: ");
  Serial.print(windDir, 2);
  Serial.println("º");
//  Serial.println(Vin, 4);

  delay(100);
}

void readRawInstance(float* data, uint32_t inputPin) {
  uint16_t raw = analogRead(inputPin);
  *data = raw * ADC_stepValue;
}

void WindDirectionInstance(float* read_Vin, float* WindDirection) {
  float R_in = R * ((Vcc / (Vcc - *read_Vin)) - 1);
  R_in /= 1000;

  uint8_t output_index = 0;
  float R_dif = abs(R_in - R_in_given[0]);
  
  for (uint8_t index = 1; index < 16; index += 1) {
    float temp_R_dif = abs(R_in - R_in_given[index]);
    if (temp_R_dif < R_dif) {
      R_dif = temp_R_dif;
      output_index = index;
    }
  }

  *WindDirection = 22.5 * output_index; 
}
