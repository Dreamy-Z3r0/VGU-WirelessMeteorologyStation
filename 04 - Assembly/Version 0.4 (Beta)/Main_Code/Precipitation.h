#include <DS3231.h>


typedef struct {
  double RainfallPerTip;
  
  unsigned long Daily_Counter;
  double Rainfall_Data;
  
  bool RainGauge_DailyAlarm;
} RainGauge_Management;

extern RainGauge_Management RainGauge;


void RainGauge_setup(uint32_t RainGauge_InputPin, uint32_t Alarm_InputPin, double RainfallPerTip);
void RainGauge_Tipping_Callback(void);
void Alarm_Callback(void);
