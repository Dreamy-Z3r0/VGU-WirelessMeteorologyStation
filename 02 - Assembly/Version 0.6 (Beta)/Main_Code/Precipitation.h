#include "DS3231.h"

#define RainfallPerTip  0.2794  // mm


/**************************************
 *** Precipitation class definition ***
 **************************************/
class Precipitation {   
  public:
    // Constructor
    Precipitation(byte hour = 9, byte minute = 0, byte second = 0);

    // Initialisation
    void init(uint32_t RainGauge_InputPin, uint32_t Alarm_InputPin);  // Initial set-ups for communicating with and managing rain gauge
    void set_DailyAlarm(byte hour = 9, byte minute = 0, byte second = 0);

    // Data-returning method
    double get_Rainfall_Data(void);   // Return the latest calculated rainfall amount

    // Internal data processing (1)
    void Increment_Counter(void);   // Increment internal count variable whenever a bucket tip appears

    // Alarm processing
    void Alarm_Presence_Processing(void);   // When there's an alarm for a new day, clear it then request for an update of rainfall
    bool is_DailyAlarm_available(void);     // When there's an alarm for a new day, return TRUE
    void clear_DailyAlarm(void);            // Clear daily alarm flag after it is used and no longer necessary

  private:
    // Class variables
    unsigned long Daily_Counter;  // Stores bucker tip counts of a day
    double Rainfall_Data;         // Stores the rainfall amount of the previous day
    bool RainGauge_DailyAlarm;    // Indicator of a new day for measuring rainfall (from 9am daily)

    byte DailyAlarm_Time[3];
    
    // Internal data processing (2)
    void Update_Rainfall_Data(void);    // Update the rainfall amount of the previous day
};


/*************************************
 *** External supporting functions ***
 *************************************/
void RainGauge_Tipping_Callback(Precipitation* PrecipitationInstance);  // Interrupt service routine when a bucket tip appears
void Alarm_Callback(Precipitation* PrecipitationInstance);              // Interrupt service routine when alarm for a new day appears
