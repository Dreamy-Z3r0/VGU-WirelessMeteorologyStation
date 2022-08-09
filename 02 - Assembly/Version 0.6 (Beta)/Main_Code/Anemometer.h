#include <Arduino.h>
#include "stdint.h"

#define KMPH_TO_MPS(KMPH) (KMPH / 3.6)
#define KMPH_TO_MPH(KMPH) (KMPH / 1.609344)

#define maxInput_windSpeed 2.38   // seconds
#define maxKnown_windSpeed 407.16 // km/h
#define dataPointsPerMeasurement 10   // Number of data points per measurement to take the final mean value

#define timeBetweenReadingPeriods 0.5   // minutes

class Anemometer_Control {
  public:
    unsigned long overflow_counter;
  
    Anemometer_Control(uint32_t Input_Pin, TIM_TypeDef *Instance_TIM = TIM1);
    void Init(void);

    void read_Wind_Speed(void);
    void Anemometer_Input_Routine(void);

    float get_Wind_Speed(void);

    void set_readFlag(void);
    bool is_readFlag_set(void);
    void clear_readFlag(void);

    bool is_idle(void);

  private:
    uint32_t Input_Pin;
    HardwareTimer *InputRoutineTim;
    
    bool readFlag, ongoingMeasurement, endOfMeasurement;

    float windSpeed;
    bool idlePeriod, calmAir;
    unsigned long Sampling_Window_Tracker;

    void Calm_Air_Detection(void);
    void Initiate_Data_Point_Read(void);
    void End_Of_Sampling_Window_Routine(float* windSpeed, uint8_t* arrayIndex);
};

void TIM_Ovf_callback(Anemometer_Control* Anemometer_Instance);
void anemometerInput_Detected(Anemometer_Control* Anemometer_Instance);
