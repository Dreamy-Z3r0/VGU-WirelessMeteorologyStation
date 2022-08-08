#define CALX_TEMP 25
#define V25       1430
#define AVG_SLOPE 4300
#define VREFINT   1200

#define ADC_resolution 12   // STM32F103 has 12-bit ADCs

#define R 3.3
#define input_WindVane PB1

#define SAMPLE_QUANTITY 50

bool samplingPeriod = false;
uint8_t minuteCounter = 0;

const float R_in_given[16] = {33.1, 6.57, 8.19, 0.89, 1, 0.69, 2.19, 1.4, 3.88, 3.12, 15.98, 14.1, 119.6, 42.07, 64.9, 21.92};
float V_in_derived[16];

uint16_t ADC_MAX_VALUE = (12 == ADC_resolution ? 4095 : 1023);

float Vcc = 3.3,      // Initial value for Vcc
      AVref = 3.3;    // Initial value for AVref

void setup() {
  Serial.begin(9600);
  analogReadResolution(ADC_resolution);   // Overwrite the default 10-bit resolution

  HardwareTimer *sensorRoutine = new HardwareTimer(TIM4);
  sensorRoutine->setOverflow(1, HERTZ_FORMAT);
  sensorRoutine->attachInterrupt(enableSensorSampling);
  sensorRoutine->resume();

  samplingPeriod = true;
}

void loop() {
  if (samplingPeriod) {
    samplingPeriod = false;

    Serial.println("\n#########");
    Serial.println("Start reading:");
    
    AVref = (VREFINT * (ADC_MAX_VALUE+1) / analogRead(AVREF)) / 1000.0;   // Take the AVref
    Vcc = AVref;

    uint16_t* rawData = new uint16_t[SAMPLE_QUANTITY];
    rawData_Sampling(rawData, SAMPLE_QUANTITY);

    Serial.print("AVref = ");
    Serial.print(AVref);
    Serial.println(" V\n");

    Serial.print("Raw data:");
    for (unsigned int i = 0; i < SAMPLE_QUANTITY; i += 1) {
      if (0 == i % 10) Serial.println(" ");
      else Serial.print(" ");

      Serial.print(*(rawData+i));
    }
    Serial.println();

    float* V_in = new float[SAMPLE_QUANTITY];
    rawData_to_V(rawData, V_in, SAMPLE_QUANTITY);
    delete[] rawData;

    Serial.print("\nV_in:");
    for (unsigned int i = 0; i < SAMPLE_QUANTITY; i += 1) {
      if (0 == i % 10) Serial.println(" ");
      else Serial.print(" ");

      Serial.print(*(V_in+i));
    }
    Serial.println();

    IIR_Filter(V_in, SAMPLE_QUANTITY);
    Serial.print("\nV_in (filtered):");
    for (unsigned int i = 0; i < SAMPLE_QUANTITY; i += 1) {
      if (0 == i % 10) Serial.println(" ");
      else Serial.print(" ");

      Serial.print(*(V_in+i));
    }
    Serial.println();
    

    float V_mean = voltage_meanValue(V_in, SAMPLE_QUANTITY);
    delete[] V_in;

    Serial.print("\nV_mean: ");
    Serial.print(V_mean);
    Serial.println(" V");
    
    for (uint8_t i = 0; i < 16; i += 1) {
      V_in_derived[i] = Vcc * R_in_given[i] / (R_in_given[i] + R);
    }

    float windDir = WindDirectionInstance(&V_mean);
    Serial.print("Wind direction: ");
    Serial.print(windDir, 1);
    Serial.println("º\n");
  }
}

void enableSensorSampling(void) {
  minuteCounter += 1;
  if (5 == minuteCounter) {
    minuteCounter = 0;
    samplingPeriod = true;
  }
}

void rawData_Sampling(uint16_t* raw_data, unsigned int arr_size) {
  for (unsigned int i = 0; i < arr_size; i += 1) {
    *(raw_data+i) = analogRead(input_WindVane);
  }
}

void rawData_to_V(uint16_t* raw_data, float* voltage, unsigned int arr_size) {
  for (unsigned int i = 0; i < arr_size; i += 1) {
    *(voltage+i) = (float)(*(raw_data+i) * AVref / ADC_MAX_VALUE);
  }
}

void IIR_Filter(float* data, unsigned int arr_size) {
  // IIR Filter direct form I output function:
  //    y[n] = (b0 * x[n] + b1 * x[n-1] + b2 * x[n-2]) - (a1 * y[n-1] + a2 * y[n-2])
  //
  // Since b2 = 0 and a2 = 0 by design, x[n-2] and y[n-2] are removed from the implementation:
  //    y[n] = (b0 * x[n] + b1 * x[n-1]) - a1 * y[n-1]

  double b0 = 1,
         b1 = 1, 
         a1 = 0.99552205147161377;

  float x = 0, y = 0;    // x[n-1] and y[n-1] 
  for (unsigned int i = 0; i < arr_size; i += 1) {
    float x_temp = data[i];    // Save the value for x[n-1]
    data[i] = (float)(b0*x_temp + b1*x - a1*y);

    x = x_temp;
    y = data[i];
  }
}

float voltage_meanValue(float* data, unsigned int arr_size) {
  float output = 0;

  for (unsigned int i = 0; i < arr_size; i += 1) {
    output += *(data+1) / arr_size;
  }

  return output;
}

float WindDirectionInstance(float* read_Vin) {
  float R_in = R * ((Vcc / (Vcc - *read_Vin)) - 1);

  Serial.print("R_in = ");
  Serial.print(R_in);
  Serial.println(" k-Ohm");

  uint8_t output_index = 0;
  float V_dif = abs(*read_Vin - V_in_derived[0]);
  Serial.print("  Index 0: ");
  Serial.print(V_dif);
  Serial.print(" => ");
  Serial.print(22.5*output_index);
  Serial.println("º");
  
  for (uint8_t index = 1; index < 16; index += 1) {
    Serial.print("  Index ");
    Serial.print(index);
    Serial.print(": ");
    
    float temp_V_dif = abs(*read_Vin - V_in_derived[index]);

    Serial.print(temp_V_dif);
    if (temp_V_dif < V_dif) {
      V_dif = temp_V_dif;
      output_index = index;

      Serial.print(" => ");
      Serial.print(22.5*output_index);
      Serial.println("º");
    }
    else Serial.println();
  }

  return (22.5 * output_index); 
}
