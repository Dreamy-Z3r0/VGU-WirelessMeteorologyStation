#include "Wind_Vane.h"


/* Global variables used in calculations */

// Maximum value of ADC output by the resolution
uint16_t ADC_MAX_VALUE = (12 == ADC_resolution ? 4095 : 1023);

// Supply voltage of the wind vane (Vcc) and ADC reference voltage of the microcontroller (AVref)
float Vcc = 3.3,      // Initial value for Vcc
      AVref = 3.3;    // Initial value for AVref


/*********************
 *** Contructor(s) ***
 *********************/

// Accept input pin parameter and pre-defined array size for temporary data storages
WindVane_Control::WindVane_Control(uint32_t ADC_input_pin, unsigned int storage_size) {
  this->ADC_input_pin = ADC_input_pin;
  this->storage_size = storage_size;

  this->readFlag = false;
}


/**************************
 *** Reading operations ***
 **************************/

// Initiate a read operation of wind direction
void WindVane_Control::Wind_Direction_Reading_Routine(void) {
  if (readFlag) {
    analogReadResolution(ADC_resolution);   // Set ADC resolution
    read_reference();   // Read the ADC reference voltage of the microcontroller
 
    uint16_t *raw_data = new uint16_t[storage_size];    // Temporary storage for ADC values
    read_raw_ADC(raw_data);    // Take ADC samples

    float* voltage_from_rawData = new float[storage_size];  // Temporary storage for voltage values converted from raw ADC data
    rawData_to_voltage(raw_data, voltage_from_rawData);     // Reconstruct voltage signals
    delete[] raw_data;    // Free up the heap

    float V_in = IIR_Mean(voltage_from_rawData);    // Apply IIR filter, then take the mean value of voltage
    delete[] voltage_from_rawData;   // Free up the heap
    
    WindDirectionInstance(V_in);     // Take the final estimation of the wind direction
  }
}

// Read the reference voltage for ADC
void WindVane_Control::read_reference(void) {
  AVref = (VREFINT * (ADC_MAX_VALUE+1) / analogRead(AVREF)) / 1000.0;   // Take the AVref
  Vcc = AVref;
}

// Sample input signal from the wind vane
void WindVane_Control::read_raw_ADC(uint16_t* storage) {  
  for (unsigned int i = 0; i < storage_size; i += 1) {
    storage[i] = analogRead(ADC_input_pin);
  }
}

// Convert raw ADC data to voltage values
void WindVane_Control::rawData_to_voltage(uint16_t* raw_data, float* voltage) {
  for (unsigned int i = 0; i < storage_size; i += 1) {
    *(voltage+i) = (AVref * (*(raw_data+i))) / ADC_MAX_VALUE;
  }
}

// Apply IIR filter and take the mean value of the filtered data
float WindVane_Control::IIR_Mean(float* data) {
  // IIR Filter direct form I output function:
  //    y[n] = (b0 * x[n] + b1 * x[n-1] + b2 * x[n-2]) - (a1 * y[n-1] + a2 * y[n-2])
  //
  // Since b2 = 0 and a2 = 0 by design, x[n-2] and y[n-2] are removed from the implementation:
  //    y[n] = (b0 * x[n] + b1 * x[n-1]) - a1 * y[n-1]

  float output = 0;
  
  double b0 = 1,
         b1 = 1, 
         a1 = 0.99552205147161377;

  float x = 0, y = 0;    // x[n-1] and y[n-1] 
  for (unsigned int i = 0; i < storage_size; i += 1) {
    float x_temp = data[i];    // Save the value for x[n-1]
    data[i] = (float)(b0*x_temp + b1*x - a1*y);

    x = x_temp;
    y = data[i];

    output += (data[i] / storage_size);
  }

  return output;
}

// Take the final estimation of the wind direction
void WindVane_Control::WindDirectionInstance(float V_in) {
  uint8_t output_index = 0;
  float V_dif;
  
  for (uint8_t index = 0; index < NumberOfDirection; index += 1) {
    float V_dir_ref = Vcc * R_in_given[index] / (R + R_in_given[index]);
    float temp_V_dif = abs(V_in - V_dir_ref);
    
    if (0 != index) {  
      if (temp_V_dif < V_dif) {
        V_dif = temp_V_dif;
        output_index = index;
      }
    }
    else {
      V_dif = temp_V_dif;
    }
  }

  windDir = 22.5 * output_index; 
}


/***********************************
 *** Data-returning operation(s) ***
 ***********************************/

// Returns the latest wind direction value
float WindVane_Control::read_Wind_Direction(void) {
  return windDir;
}


/*****************************
 *** readFlag operation(s) ***
 *****************************/

// Set readFlag
void WindVane_Control::set_readFlag(void) {
  readFlag = true;
}

// Return value
bool WindVane_Control::is_readFlag_set(void) {
  return readFlag;
}

// Clear readFlag
void WindVane_Control::clear_readFlag(void) {
  readFlag = false;
}
