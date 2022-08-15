#define CALX_TEMP 25
#define V25       1430
#define AVG_SLOPE 4300
#define VREFINT   1200
#define ADC_RANGE 4096

#define ADC_Input PB1
#define numberOfDataPoints 50

float AVref = 3.3;

uint16_t ADC_read_data[numberOfDataPoints];
float voltage[numberOfDataPoints];
bool ADC_start = false;

uint16_t ADC_filtered_data[numberOfDataPoints];
float voltage_filtered_ADC[numberOfDataPoints];
float voltage_filtered[numberOfDataPoints];

void setup() {
  Serial.begin(9600);
  analogReadResolution(12);
}

void loop() {
  float voltage_multimeter;
  if (Serial.available()) {
    String input = Serial.readStringUntil('\n');

    int8_t separator = input.indexOf(':');
    if (-1 != separator) {
      if (input.substring(0,separator).equals("start")) {
        voltage_multimeter = input.substring(separator+1).toFloat();
        ADC_start = true;
      }
    }
  }

  if (ADC_start) {
    Serial.println("/* New loop */\n");
    
    ADC_start = false;
    AVref = (VREFINT * ADC_RANGE / analogRead(AVREF)) / 1000.0;   // Take the AVref
    
    take_ADC_samples(ADC_read_data, numberOfDataPoints);    // Take ADC samples
    Serial.print("Raw ADC samples:\n  ");           // Print raw ADC samples
    print_ADC(ADC_read_data, numberOfDataPoints);

    voltage_conversion(ADC_read_data, voltage, numberOfDataPoints);   // Convert from raw ADC data to voltage
    Serial.print("Voltage from raw ADC data:\n  ");   // Print voltage values after the conversion
    print_Voltage(voltage, numberOfDataPoints);

    IIR_DirectForm_I(ADC_read_data, ADC_filtered_data, numberOfDataPoints);   // Apply IIR filter for raw ADC data
    Serial.print("Filtered ADC data:\n  ");           // Print filtered ADC data
    print_ADC(ADC_filtered_data, numberOfDataPoints);

    voltage_conversion(ADC_filtered_data, voltage_filtered_ADC, numberOfDataPoints);   // Convert from filtered ADC data to voltage
    Serial.print("Voltage from filtered ADC data:\n  ");   // Print voltage values after the conversion
    print_Voltage(voltage_filtered_ADC, numberOfDataPoints);

    IIR_DirectForm_II(voltage, voltage_filtered, numberOfDataPoints);
    Serial.print("Voltage filtered after converions from raw ADC data:\n  ");   // Print filtered voltage values after the conversion from raw ADC data
    print_Voltage(voltage_filtered, numberOfDataPoints);

    Serial.println();

    Serial.print("Voltage from multimeter: ");
    Serial.println(voltage_multimeter, 3);
    Serial.println();

    uint16_t meanADCdata;
    float meanVoltage;
  
    mean_ADC_data(ADC_read_data, &meanADCdata, numberOfDataPoints);
    voltage_conversion(&meanADCdata, &meanVoltage, 1);
    Serial.print("Mean read ADC data: ");
    Serial.println(meanADCdata);
    Serial.print("Voltage from mean read ADC data: ");
    Serial.print(meanVoltage);
    Serial.print(" (");
    Serial.print(meanVoltage-voltage_multimeter, 3);
    Serial.println(")");
    Serial.println();

    mean_ADC_data(ADC_filtered_data, &meanADCdata, numberOfDataPoints);
    voltage_conversion(&meanADCdata, &meanVoltage, 1);
    Serial.print("Mean filtered ADC data: ");
    Serial.println(meanADCdata);
    Serial.print("Voltage from mean filtered ADC data: ");
    Serial.print(meanVoltage);
    Serial.print(" (");
    Serial.print(meanVoltage-voltage_multimeter, 3);
    Serial.println(")");
    Serial.println();
    
    mean_voltage(voltage, &meanVoltage, numberOfDataPoints);
    Serial.print("Mean voltage from raw ADC data: ");
    Serial.print(meanVoltage, 3);
    Serial.print(" (");
    Serial.print(meanVoltage-voltage_multimeter, 3);
    Serial.println(")");
    mean_voltage(voltage_filtered_ADC, &meanVoltage, numberOfDataPoints);
    Serial.print("Mean voltage from filtered ADC data: ");
    Serial.print(meanVoltage, 3);
    Serial.print(" (");
    Serial.print(meanVoltage-voltage_multimeter, 3);
    Serial.println(")");
    mean_voltage(voltage_filtered, &meanVoltage, numberOfDataPoints);
    Serial.print("Mean filtered voltage from raw ADC data: ");
    Serial.print(meanVoltage, 3);
    Serial.print(" (");
    Serial.print(meanVoltage-voltage_multimeter, 3);
    Serial.println(")");

    Serial.println("\n");
  }
}

void mean_ADC_data(uint16_t* ADC_arr, uint16_t* output, uint8_t arraySize) {
  double output_temp = 0;

  for (uint8_t i = arraySize; i > 0; i -= 1) {
    output_temp += ADC_arr[i-1] / (1.0 * arraySize);
  }

  *output = (uint16_t)output_temp;
}

void mean_voltage(float* voltage_arr, float* output, uint8_t arraySize) {
  *output = 0;

  for (uint8_t i = arraySize; i > 0; i -= 1) {
    *output += voltage_arr[i-1] / arraySize;
  }
}

void take_ADC_samples(uint16_t* ADC_arr, uint8_t arraySize) {   // ADC sampling method
  for (uint8_t i = 0; i < arraySize; i += 1) {
    ADC_arr[i] = analogRead(ADC_Input);
  }
}

void voltage_conversion(uint16_t* ADC_arr, float* voltage_arr, uint8_t arraySize) {   // Voltage conversion method
  for (uint8_t i = 0; i < arraySize; i += 1) {
    if (0 == ADC_arr[i])
      voltage_arr[i] = 0;
    else
      voltage_arr[i] = (AVref * ADC_arr[i]) / (ADC_RANGE - 1);
  }
}

void print_ADC(uint16_t* ADC_arr, uint8_t arraySize) {
  for (uint8_t i = 0; i < arraySize; i += 1) {
    Serial.println(ADC_arr[i]);
//    Serial.print(" ");
  }
  Serial.println();
}

void print_Voltage(float* voltage_arr, uint8_t arraySize) {
  for (uint8_t i = 0; i < arraySize; i += 1) {
    Serial.print(voltage_arr[i], 3);
    Serial.print(" ");
  }
  Serial.println();
}

void IIR_DirectForm_I(uint16_t* ADC_arr, uint16_t* output_arr, uint8_t arraySize) {   // IIR filter for fixed point implementation
  double b0 = 1,
         b1 = 1, 
         a1 = 0.92218606833315153;
         
  for (uint8_t i = 0; i < arraySize; i += 1) {
    if (0 == i) {
      output_arr[i] = b0 * ADC_arr[i];
    }
    else {
      output_arr[i] = (uint16_t)(b0 * ADC_arr[i] + b1 * ADC_arr[i-1] - a1 * output_arr[i-1]);
    }
  }
}

void IIR_DirectForm_II(float* voltage_arr, float* output_arr, uint8_t arraySize) {   // IIR filter for floating point implementation
  double b0 = 1,
         b1 = 1, 
         a1 = 0.92218606833315164;
       
  for (uint8_t i = 0; i < arraySize; i += 1) {
    output_arr[i] = b0 * voltage_arr[i];

    if (0 == i) {
      output_arr[i] = b0 * voltage_arr[i];
    }
    else {
      output_arr[i] = (float)(b0 * voltage_arr[i] + b1 * voltage_arr[i-1] - a1 * output_arr[i-1]);
    }
  }
}
