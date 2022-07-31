#include "Wind_Monitoring.h"

uint16_t ADC_MAX_VALUE = (12 == ADC_resolution ? 4095 : 1023);

float Vcc = 3.3,      // Initial value for Vcc
      AVref = 3.3;    // Initial value for AVref


WindVane_Control::WindVane_Control(uint32_t ADC_input_pin, unsigned int storage_size) {
  this->ADC_input_pin = ADC_input_pin;
  this->storage_size = storage_size;

  readFlag = false;
}

float WindVane_Control::read_Wind_Direction(void) {
  if (readFlag) {
    uint16_t *raw_data = new uint16_t[storage_size];
  
    read_raw_ADC(raw_data);    // Take ADC samples
    RawDataProcessing(raw_data);        // Process raw data: digital filter -> reconstruct voltage signal -> take mean voltage value
    WindDirectionInstance();

    delete[] raw_data;

    return windDir;
  }
}

void WindVane_Control::read_raw_ADC(uint16_t* storage) {
  AVref = (VREFINT * (ADC_MAX_VALUE+1) / analogRead(AVREF)) / 1000.0;   // Take the AVref
  
  for (unsigned int i = 0; i < storage_size; i += 1) {
    storage[i] = analogRead(ADC_input_pin);
  }
}

void WindVane_Control::RawDataProcessing(uint16_t* raw_data) {
  float reconstructed_signal;
  
  IIR_Filter(raw_data);    // Apply IIR filter direct form I for raw ADC values

  V_in = 0;
  for (unsigned int i = 0; i < storage_size; i += 1) {   
    reconstructed_signal = (float)((AVref * raw_data[i]) / ADC_MAX_VALUE);    // Reconstruct voltage signal from each filtered ADC input
    V_in += reconstructed_signal / storage_size;    // Take mean value of reconstructed voltage signals
  }
}

void WindVane_Control::WindDirectionInstance(void) {
  R_in = R * ((Vcc / (Vcc - V_in)) - 1);
  R_in /= 1000;

  uint8_t output_index = 0;
  float R_dif = abs(R_in - R_in_given[0]);
  
  for (uint8_t index = 1; index < NumberOfDirection; index += 1) {
    float temp_R_dif = abs(R_in - R_in_given[index]);
    if (temp_R_dif < R_dif) {
      R_dif = temp_R_dif;
      output_index = index;
    }
  }

  windDir = 22.5 * output_index; 
}

void WindVane_Control::IIR_Filter(uint16_t* data) {
  // IIR Filter direct form I
  //    y[n] = (b0 * x[n] + b1 * x[n-1] + b2 * x[n-2]) - (a1 * y[n-1] + a2 * y[n-2])
  //
  // Since b2 = 0 and a2 = 0 by design, x[n-2] and y[n-2] are removed from the implementation:
  //    y[n] = (b0 * x[n] + b1 * x[n-1]) - a1 * y[n-1]

  double b0 = 1,
         b1 = 1, 
         a1 = 0.92218606833315153;

  uint16_t x = 0, y = 0;    // x[n-1] and y[n-1] 
  for (unsigned int i = 0; i < storage_size; i += 1) {
    uint16_t x_temp = data[i];    // Save the value for x[n-1]
    data[i] = (uint16_t)(b0*x_temp + b1*x - a1*y);

    x = x_temp;
    y = data[i];
  }
}

void WindVane_Control::set_readFlag(void) {
  readFlag = true;
}

bool WindVane_Control::is_readFlag_set(void) {
  return readFlag;
}

void WindVane_Control::clear_readFlag(void) {
  readFlag = false;
}
