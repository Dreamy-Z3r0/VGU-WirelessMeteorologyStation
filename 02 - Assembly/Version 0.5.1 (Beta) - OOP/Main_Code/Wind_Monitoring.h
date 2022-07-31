#include "Arduino.h"

#define CALX_TEMP 25
#define V25       1430
#define AVG_SLOPE 4300
#define VREFINT   1200

#define numberOfDataPoints 50

#define ADC_resolution 12   // STM32F103 has 12-bit ADCs
#define R 3.3E3

#define NumberOfDirection 16
const float R_in_given[] = {33.1, 6.57, 8.19, 0.89, 1, 0.69, 2.19, 1.4, 3.88, 3.12, 15.98, 14.1, 119.6, 42.07, 64.9, 21.92};

class WindVane_Control {
  public:
    WindVane_Control(uint32_t ADC_input_pin, unsigned int storage_size = numberOfDataPoints);

    float read_Wind_Direction(void);
    void read_raw_ADC(uint16_t* storage);
    void RawDataProcessing(uint16_t* raw_data);
    void WindDirectionInstance(void);

    void set_readFlag(void);
    bool is_readFlag_set(void);
    void clear_readFlag(void);

  private:
    uint32_t ADC_input_pin;
    float V_in, R_in, windDir;
    unsigned int storage_size;

    bool readFlag;

    void IIR_Filter(uint16_t* data);
};
