#ifndef _PRECIPITATION_H_
#define _PRECIPITATION_H_

// #include <DS3231.h>
#include "Sensor_Base.h"

#define RainfallPerTip  0.2794  // mm


/**************************************
 *** Precipitation class definition ***
 **************************************/
class Precipitation : public Sensor_Base {   
  public:
    // Constructor
    Precipitation(void);
    Precipitation(uint32_t SensorPin);
    
    // Initialisation
    void init(void);  // Initial setups for communicating with and managing rain gauge
    
    // Update the rainfall amount of the previous day
    void update_sensor_data(void);

    // Data-returning method
    void read_sensor_data(float *external_storage);   // Return the latest calculated rainfall amount

    // Internal operation(s) for external interrupt service routine(s)
    void Increment_Counter(void);       // Increment internal count whenever the bucket tips

    // Alarm processing
    void Alarm_Presence_Processing(void);   // When there's an alarm for a new day, clear it then request for an update of rainfall

  private:
    // Class variables
    unsigned long Daily_Counter;  // Stores bucker tip counts of a day
    float Rainfall_Data;          // Stores the rainfall amount of the previous day
};


/*******************************************
 *** External interrupt service routines ***
 *******************************************/
void RainGauge_Tipping_Callback(Precipitation* PrecipitationInstance);  // Interrupt service routine when the bucket tips

#endif