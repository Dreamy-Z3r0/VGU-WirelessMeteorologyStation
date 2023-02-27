#include <Arduino.h>
#include "stdint.h"

#define NOP __asm__ __volatile__ ("nop\n\t")

#define KMPH_TO_MPS(KMPH) (KMPH / 3.6)
#define KMPH_TO_MPH(KMPH) (KMPH / 1.609344)

#define maxInput_windSpeed 2.38   // seconds
#define maxKnown_windSpeed 407.16 // km/h
#define dataPointsPerMeasurement 10   // Number of data points per measurement to take the final mean value

#define timeBetweenReadingPeriods 0.5   // minutes

class Anemometer_Control {
  public:
    Anemometer_Control(uint32_t Input_Pin);

    void Init(TIM_TypeDef* EdgePeriodTimer_Instance = TIM1, TIM_TypeDef* CalmAirTimer_Instance = TIM2);

    void Anemometer_Reading_Routine(void);
    float read_Wind_Speed(void);

    void set_readFlag(void);
    bool is_readFlag_set(void);
    void clear_readFlag(void);

    bool is_idlePeriod(void);

    void Timer_Callback(HardwareTimer* OverflownTimer);
    void Input_Callback(void);
    
  private:
    uint32_t Input_Pin;
    unsigned long idleTimeInMilliseconds;
    
    HardwareTimer* EdgePeriodTimer;
    HardwareTimer* CalmAirTimer;
    
    bool idlePeriod, readFlag;
    bool isTakingFirstEdge, isSecondEdgeDetected;
    bool CalmAir;

    unsigned long EdgeTiming_Overflow;
    uint8_t CalmAirTiming_Overflow,
            CalmAirTiming_MaxValue;

    float windSpeed[dataPointsPerMeasurement];
    uint8_t arr_index;

    uint8_t fault_count;
    float meanWindSpeed;

    void Initialise_New_Timing_Period(void);
    void WindSpeed_Array_Update_Routine(void);
    void WindSpeed_MeanValue_Update_Routine(void);
};

void TIM_Ovf_callback(Anemometer_Control* Anemometer_Instance, HardwareTimer* OverflownTimer);
void anemometerInput_Detected(Anemometer_Control* Anemometer_Instance);
