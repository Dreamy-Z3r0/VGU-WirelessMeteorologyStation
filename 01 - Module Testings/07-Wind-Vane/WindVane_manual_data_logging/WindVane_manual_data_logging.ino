#include "SPI.h"
#include "SD.h"

#define V_ref 3.26          // ADC reference voltage
#define ADC_resolution 12   // STM32F103 has 12-bit ADCs
float   ADC_stepValue;      // Step value = V_ref / (2^ADC_resolution - 1)

#define Vcc 3.3
#define R 3.3E3
#define input_WindVane PB1

float R_in_given[] = {33.1, 6.57, 8.19, 0.89, 1, 0.69, 2.19, 1.4, 3.88, 3.12, 15.98, 14.1, 119.6, 42.07, 64.9, 21.92};

#define CSPin PB12
File cardData;
String fileName = "";
#define inputsPerRequest 20

void setup() {
  Serial.begin(9600);

  SPI.setMOSI(PB15);
  SPI.setMISO(PB14);
  SPI.setSCLK(PB13);
  SPI.setSSEL(PB12);
  pinMode(CSPin, OUTPUT);

   if (!SD.begin(CSPin)) {
    return;     // do nothing more
   }

  String baseName = "Wind_vane_";
  String extension = ".csv";
  int nameIndex = 0;
  
  bool existedFile = true;
  while (existedFile) {
    fileName = baseName + String(nameIndex) + extension;
    if (SD.exists(fileName)) {
      nameIndex += 1;
    } else {
      existedFile = false;
      cardData = SD.open(fileName, FILE_WRITE);
      cardData.println("Index,Raw data,v_read,R_in,Dir_calculated,Dir_observed");
      cardData.close();
    }
  }

  analogReadResolution(ADC_resolution);   // Overwrite the default 10-bit resolution
  ADC_stepValue = V_ref / (pow(2,ADC_resolution)-1);
}

void loop() {
  String serialInput = "";
  static bool loggingRequest = false;
  static int loggingIndex = 0;
  static int indexCounter = 0;
  
  if (Serial.available()) {
    serialInput = Serial.readStringUntil('\n');
    loggingRequest = true;
  }

  uint16_t rawData;
  float voltageValue, resistanceValue, windDirection;

  readRawInstance(&rawData, &voltageValue, input_WindVane);
  WindDirectionInstance(&voltageValue, &windDirection);

  if (loggingRequest) {
    cardData = SD.open(fileName, FILE_WRITE);
    
    cardData.print(loggingIndex);
    cardData.print(".");
    cardData.print(indexCounter);
    cardData.print(",");

    cardData.print(rawData);
    cardData.print(",");

    cardData.print(voltageValue);
    cardData.print(",");

    cardData.print(resistanceValue);
    cardData.print(",");

    cardData.print(windDirection);
    cardData.print(",");

    cardData.print(serialInput);
    cardData.println(",");

    cardData.close();
    
    indexCounter += 1;
    if (inputsPerRequest == indexCounter) {
      indexCounter = 0;
      loggingIndex += 1;
      loggingRequest = false;
    }
  } else {
    Serial.print("Wind direction: ");
    Serial.print(windDirection, 2);
    Serial.println("º");

    Serial.print("    Raw value: ");
    Serial.println(rawData);

    Serial.print("    => ");
    Serial.print(voltageValue, 6);
    Serial.print(" V  /  ");
    Serial.print(resistanceValue, 2);
    Serial.println(" Ohm");

    delay(100);
  }
}

void readRawInstance(uint16_t* raw, float* data, uint32_t inputPin) {
  *raw = analogRead(inputPin);
  *data = *raw * ADC_stepValue;
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
