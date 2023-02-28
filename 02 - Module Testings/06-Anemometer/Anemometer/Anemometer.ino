#define input_Anemometer PB2
#define numberOfPoints 10

unsigned long int period_storage[numberOfPoints+1];
unsigned int storage_index = 0;
bool data_ready = false;

#define anemometerRadius 9.0
float windSpeed;

void setup() {
  Serial.begin(9600);
  attachInterrupt(digitalPinToInterrupt(input_Anemometer), anemometer_isr, FALLING);
}

void loop() {
  if (data_ready) {
     detachInterrupt(digitalPinToInterrupt(input_Anemometer));
     dataProcessing(&windSpeed);

     Serial.print("Wind speed: ");
     Serial.print(windSpeed, 3);
     Serial.println(" m/s");

     delay(1000);
     data_ready = false;
     attachInterrupt(digitalPinToInterrupt(input_Anemometer), anemometer_isr, FALLING);
  }
}

void anemometer_isr() {
  if (!data_ready) {
    period_storage[storage_index] = micros();
    
    storage_index += 1;
    if (storage_index > numberOfPoints) {
      storage_index = 0;
      data_ready = true;
    }
  }
}

void dataProcessing(float* output) {
  period_storage[0] = period_storage[1] - period_storage[0];
  
  for (uint8_t index = 1; index < numberOfPoints; index += 1) {
    period_storage[0] += period_storage[index+1] - period_storage[index];
  }

  float rev_period = period_storage[0] / (float)numberOfPoints;

  *output = (10000 * TWO_PI * anemometerRadius) / rev_period;
}
