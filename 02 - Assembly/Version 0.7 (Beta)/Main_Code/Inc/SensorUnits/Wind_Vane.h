#pragma once

/* STM32F1xx specific HAL configuration options. */
#if __has_include("hal_conf_custom.h")
#include "hal_conf_custom.h"
#else
#if __has_include("hal_conf_extra.h")
#include "hal_conf_extra.h"
#endif
#include "stm32f1xx_hal_conf_default.h"
#endif

#include "../Sensor_Base.h"

/* Constants from datasheet */
#define CALX_TEMP 25      // Reference temperature (25ºC)
#define V25       1430    // Vnternal voltage signal (V_sense) at reference temperature in mV
#define AVG_SLOPE 4300    // Average slope for curve between temperature and V_sense
#define VREFINT   1200    // Internal reference voltage in mV

enum ADC_INPUT_TYPE {INTERNAL_REFERENCE_VOLTAGE, EXTERNAL_INPUT_SIGNAL};

extern ADC_HandleTypeDef hadc1;
extern DMA_HandleTypeDef hdma_adc1;

#define numberOfDataPoints 50

#define R 3.3   // k-Ohm

#define NumberOfDirection 16
const float R_in_given[] = {33.1, 6.57, 8.19, 0.89, 1, 0.69, 2.19, 1.4, 3.88, 3.12, 15.98, 14.1, 119.6, 42.07, 64.9, 21.92};

class WindVane_Control : public Sensor_Base {
  public:
    // Class constructor(s)
    WindVane_Control(unsigned int storage_size = numberOfDataPoints);
    WindVane_Control(uint32_t ADC_input_pin, unsigned int storage_size = numberOfDataPoints);

    // Public operation(s)
    void init(TIM_TypeDef* StandbyTimer_Instance = TIM3);    // Initiate the ADC

    void update_standby(unsigned long standby_period = 50);  
    
    void update_sensor_data(void);    // Initiate a read operation of wind direction
    void End_Of_Sampling_Routine(bool half_complete = false);

    bool is_Data_Ready(void);
    void read_sensor_data(float *external_storage);    // Returns the latest wind direction value

    void Timer_Callback(void);

  private:
    float windDir;    // Latest wind direction value
    unsigned int storage_size;  // Array size for temporary data storages

    uint16_t* raw_data;

    bool half_complete, // Indicates that ADC is on-going and the buffer is half-filled
         sample_ready;  // Indicates the end of a sampling routine

    // IIR filter coefficients
    double b0 = 1,
           b1 = 1, 
           b2 = 0,        
           a1 = 0.99919607551456557000,
           a2 = 0; 

    HardwareTimer* StandbyTimer;
    unsigned int standby_period, standby_counter;

    void read_reference(void);              // Read the reference voltage for ADC
    void read_raw_ADC(uint16_t* storage);   // Sample input signal from the wind vane

    void Data_Processing_Routine(uint16_t* raw_data);     // Process raw ADC input and update wind direction
    
    float rawData_to_voltage(uint16_t* rawData);   // Convert raw ADC data to voltage values
    void IIR_Filter(float* x, float* y);           // Apply IIR filter

    void WindDirectionInstance(float V_in);   // Take the final estimation of the wind direction
};

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void);

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(ADC_INPUT_TYPE input_type);

void STANDBY_TIM_Ovf_callback(WindVane_Control *WindVane_Instance);

extern WindVane_Control WindVane;
