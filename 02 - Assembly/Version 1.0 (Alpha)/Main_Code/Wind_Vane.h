#include "Arduino.h"

/* Constants from datasheet */
#define CALX_TEMP 25      // Reference temperature (25ºC)
#define V25       1430    // Vnternal voltage signal (V_sense) at reference temperature in mV
#define AVG_SLOPE 4300    // Average slope for curve between temperature and V_sense
#define VREFINT   1200    // Internal reference voltage in mV

#define numberOfDataPoints 50

#define ADC_resolution 12   // STM32F103 has 12-bit ADCs
#define R 3.3

#define NumberOfDirection 16
const float R_in_given[] = {33.1, 6.57, 8.19, 0.89, 1, 0.69, 2.19, 1.4, 3.88, 3.12, 15.98, 14.1, 119.6, 42.07, 64.9, 21.92};

class WindVane_Control {
  public:
    // Class constructor(s)
    WindVane_Control(uint32_t ADC_input_pin, unsigned int storage_size = numberOfDataPoints);

    // Public operation(s)
    void Wind_Direction_Reading_Routine(void);    // Initiate a read operation of wind direction
    float read_Wind_Direction(void);              // Returns the latest wind direction value

    // readFlag operations
    void set_readFlag(void);      // Set readFlag
    bool is_readFlag_set(void);   // Return value
    void clear_readFlag(void);    // Clear readFlag

  private:
    uint32_t ADC_input_pin;     // Wind vane input pin
    float windDir;    // Latest wind direction value
    unsigned int storage_size;  // Array size for temporary data storages

    bool readFlag;    // Enable to issue a wind direction reading

    void read_reference(void);              // Read the reference voltage for ADC
    void read_raw_ADC(uint16_t* storage);   // Sample input signal from the wind vane

    void rawData_to_voltage(uint16_t* rawData, float* voltage);   // Convert raw ADC data to voltage values
    float IIR_Mean(float* data);                                  // Apply IIR filter and take the mean value of the filtered data

    void WindDirectionInstance(float V_in);   // Take the final estimation of the wind direction
};
