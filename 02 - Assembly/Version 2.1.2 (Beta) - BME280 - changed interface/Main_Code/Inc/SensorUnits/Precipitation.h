#ifndef _PRECIPITATION_H_
#define _PRECIPITATION_H_

#include <DS3231.h>
#include "Sensor_Base.h"

#define RainfallPerTip  0.2794  // mm


/**************************************
 *** Precipitation class definition ***
 **************************************/
class Precipitation : public Sensor_Base {   
  public:
    // Constructor
    Precipitation(void);
    Precipitation(uint32_t SensorPin, uint32_t Alarm_InputPin);
    Precipitation(uint32_t SensorPin, uint32_t Alarm_InputPin, byte hour, byte minute, byte second);

    // Initialisation
    void init(void);  // Initial setups for communicating with and managing rain gauge
    void set_DailyAlarm(byte hour = 9, byte minute = 0, byte second = 0);   // Set daily alarm (Default: 9 A.M)

    // Data-returning method
    void read_sensor_data(float *external_storage);   // Return the latest calculated rainfall amount

    // Internal operation(s) for external interrupt service routine(s)
    void Increment_Counter(void);       // Increment internal count whenever the bucket tips

    // Alarm processing
    void Alarm_Presence_Processing(void);   // When there's an alarm for a new day, clear it then request for an update of rainfall
    bool is_DailyAlarm_available(void);     // When there's an alarm for a new day, return TRUE
    void clear_DailyAlarm(void);            // Clear daily alarm flag after it is used and no longer necessary

  private:
    // Class variables
    unsigned long Daily_Counter;  // Stores bucker tip counts of a day
    float Rainfall_Data;          // Stores the rainfall amount of the previous day
    bool RainGauge_DailyAlarm;    // Indicator of a new day for measuring rainfall (from 9am daily)

    uint32_t Alarm_InputPin;      // DS33231 SQW pin serving as alarm trigger
    byte DailyAlarm_Time[3];      // Stores daily alarm value
    
    // Internal data processing
    void Update_Rainfall_Data(void);    // Update the rainfall amount of the previous day
};


/*******************************************
 *** External interrupt service routines ***
 *******************************************/
void RainGauge_Tipping_Callback(Precipitation* PrecipitationInstance);  // Interrupt service routine when the bucket tips
void Alarm_Callback(Precipitation* PrecipitationInstance);              // Interrupt service routine when alarm for a new day appears

#endif