#include "Wind_Monitoring.h"


void read_Wind_Direction(float* V_in, float* R_in, float* windDir, uint32_t ADC_input_pin) {
  read_raw_ADC(raw_data, ADC_input_pin, numberOfDataPoints);    // Take ADC samples
  RawDataProcessing(raw_data, V_in, numberOfDataPoints);        // Process raw data: digital filter -> take mean value -> 
  
}

void read_raw_ADC(uint16_t* storage, uint32_t ADC_input_pin, unsigned int storage_size) {
  for (unsigned int i = 0; i < storage_size; i += 1) {
    storage[i] = analogRead(ADC_input_pin);
  }
}

void RawDataProcessing(uint16_t* raw_data, float* voltage_output, unsigned int InputQuantity) {
  IIR_Filter(raw_data, InputQuantity);    // Apply IIR filter direct form I for raw ADC values

  float *reconstructed_signals[
  for (unsigned int i = 0; i < InputQuantity; i += 1) {   // Take mean value
    mean_rawData += (raw_data[i] / (1.0 * InputQuantity));
  }

  *voltage_output = (float)((AVref * mean_rawData) / (pow(2, ADC_resolution) - 1));   // Convert from mean filtered ADC data to voltage
}

void WindDirectionInstance(float* R_in, float* V_in, float* windDir) {
  *R_in = R * ((Vcc / (Vcc - *V_in)) - 1);
  *R_in /= 1000;

  uint8_t output_index = 0;
  float R_dif = abs(*R_in - R_in_given[0]);
  
  for (uint8_t index = 1; index < NumberOfDirection; index += 1) {
    float temp_R_dif = abs(*R_in - R_in_given[index]);
    if (temp_R_dif < R_dif) {
      R_dif = temp_R_dif;
      output_index = index;
    }
  }

  *windDir = 22.5 * output_index; 
}

void IIR_Filter(uint16_t* data, unsigned int storage_size) {
  // IIR Filter direct form I
  //    y[n] = (b0 * x[n] + b1 * x[n-1] + b2 * x[n-2]) - (a1 * y[n-1] + a2 * y[n-2])
  //
  // Since b2 = 0 and a2 = 0 by design, x[n-2] and y[n-2] are removed from the implementation:
  //    y[n] = (b0 * x[n] + b1 * x[n-1]) - a1 * y[n-1]

  double b0 = 1,
         b1 = 1, 
         a1 = 0.92218606833315153;

  uint16_t x = 0, y = 0;    // x[n-1] and y[n-1] 
  for (unsigned int i = 0; i < arraySize; i += 1) {
    uint16_t x_temp = data[i];    // Save the value for x[n-1]
    data[i] = (uint16_t)(b0*x_temp + b1*x - a1*y);

    x = x_temp;
    y = data[i];
  }
}
