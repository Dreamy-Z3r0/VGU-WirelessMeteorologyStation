#pragma once

#include "../Sensor_Base.h"


/* Speed conversion methods */
// Note: KMPH - kilometers per hour (km/h)
//        MPS - meters per second (m/s)
//        MPH - miles per hour (mph)

#define KMPH_TO_MPS(KMPH) (KMPH / 3.6)        // km/h -> m/s
#define KMPH_TO_MPH(KMPH) (KMPH / 1.609344)   // km/h -> mph

#define MPS_TO_KMPH(MPS)  (MPS * 3.6)   // m/s -> km/h
#define MPS_TO_MPH(MPS)   (KMPH_TO_MPH(MPS_TO_KMPH(MPS)))   // m/s -> mph

#define MPH_TO_KMPH(MPH)  (MPH * 1.609344)    // mph -> km/h
#define MPH_TO_MPS(MPH)   (KMPH_TO_MPS(MPH_TO_KMPH(MPH)))   // mph -> m/s


/* Pre-defined constants */
#define maxInput_windSpeed 2.4    // seconds -> highest duration between 2 input edges for light air (wind speed of 1 km/h, Beaufort Wind Scale)
#define maxKnown_windSpeed 407.16 // km/h -> highest wind speed ever recorded on Earth by June 2022
#define dataPointsPerMeasurement 10   // Number of data points per measurement to take the final mean value

#define timeBetweenTwoReadingRoutines 30000   // ms -> standby period between 2 consecutive readings


/***********************************
 *** Anemometer class definition ***
 ***********************************/
class Anemometer_Control : public Sensor_Base {
  public:
    // Class constructor(s)
    Anemometer_Control(void);
    Anemometer_Control(uint32_t SensorPin);

    // Device initialization
    void init(TIM_TypeDef* AnemometerTimer_Instance = TIM1);

    // Public operations
    void update_sensor_data(void);    // Continuously called to perform wind speed reading
    void read_sensor_data(float *external_storage);    // Returns the latest wind speed value

    // Internal operations for external interrupt service routines
    void Timer_Callback(void);   // Timer counter overflow callback
    void Input_Callback(void);    // Edge presence callback
    
  private:
    HardwareTimer* AnemometerTimer;   // Timer for anemometer
    
    bool isTakingFirstEdge, isSecondEdgeDetected;  // Edge-controlling flags 
    bool CalmAir;   // Calm air detection

    unsigned long Timing_Overflow;    // Helps timing edges since duration between 2 edges could exceed timer counter capacity
    uint8_t CalmAirTiming_MaxValue;   // Max value for CalmAirTiming_Overflow by which a calm air period is determined

    float windSpeed[dataPointsPerMeasurement];    // Stores wind speed data points of a reading
    uint8_t arr_index;    // Index for windSpeed[]

    uint8_t fault_count;    // Number of faults out of dataPointsPerMeasurement detected within a single reading period
    float meanWindSpeed;    // Latest valid mean value of wind speed

    void Initialise_New_Timing_Period(void);          // Initiate a timing period for each data point of windSpeed[]
    void WindSpeed_Array_Update_Routine(void);        // Update windSpeed[] at the end of a timing period
    void WindSpeed_MeanValue_Update_Routine(void);    // Calculate meanWindSpeed at the end of a reading period (arr_index reaches max value)
};


/*******************************************
 *** External interrupt service routines ***
 *******************************************/
void Anemometer_Control_TIM_Ovf_Callback(Anemometer_Control* Anemometer_Instance);    // Interrupt service routine when a timer counter overflows
void anemometerInput_Detected(Anemometer_Control* Anemometer_Instance);   // Interrupt service routine when an edge is present at the input pin
