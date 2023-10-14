#ifndef _ANEMOMETER_H_
#define _ANEMETER_H_

#include "Sensor_Base.h"


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


/* Pre-defined functions/constants */
// #define ConstantAnemometerFactor 1.8


/* Pre-defined constants */
#define AnemometerRadius 0.09  // m; Radius of the anemometer from the centre of the rotor 
                               // to the outer edge of a cup

#define WindDataUpdateInterval 5   // minutes; Wind speed data is updated every 5 or 10 minutes 
                                   // to minimise the effects of wind gusts.


/***********************************
 *** Anemometer class definition ***
 ***********************************/
class Anemometer_Control : public Sensor_Base {
  public:
    // Class constructor(s)
    Anemometer_Control(void);
    Anemometer_Control(uint32_t SensorPin);

    // Device initialization
    void init(void);

    // Public operations
    void update_sensor_data(void);    // Initiate a read operation of wind speed
    void read_sensor_data(float *external_storage);    // Returns the latest wind speed value

    // Internal operations for external interrupt service routine(s)
    void Input_Callback(void);    // Edge presence callback
    
  private:
    int count;              // Number of detected edges from anemometer during a observation interval
    float meanWindSpeed;    // Latest valid mean value of wind speed

    float wind_speed_conversion(int pulseQuantity);    // Calculate wind speed from recorded number of edges
    float AnemometerFactor(float readWindSpeed);
};


/*********************************************
 *** External interrupt service routine(s) ***
 *********************************************/
void anemometerInput_Detected(Anemometer_Control* Anemometer_Instance);   // Interrupt service routine when an edge is present at the input pin

#endif
