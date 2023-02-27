#include "Precipitation.h"


RainGauge_Management RainGauge;


void RainGauge_setup(uint32_t RainGauge_InputPin, uint32_t Alarm_InputPin, double RainfallPerTip) {
  RainGauge.RainfallPerTip = RainfallPerTip;
  RainGauge.Daily_Counter = 0;
  
  pinMode(RainGauge_InputPin, INPUT);  // Initialize rain gauge input
  pinMode(Alarm_InputPin, INPUT_PULLUP);      // Initialize alarm pin

  attachInterrupt(digitalPinToInterrupt(RainGauge_InputPin), RainGauge_Tipping_Callback, FALLING);   // Routine on a bucket tip

  DS3231 clock;
  clock.setA1Time(1, 9, 0, 0, 0x8, false, false, false);    // Set up daily alarm at 9 am sharp
  clock.turnOnAlarm(1);     // Enable alarm 1

  if (!digitalRead(Alarm_InputPin)) {   // Make sure there is no alarm present initially
    clock.checkIfAlarm(1);
  }

  attachInterrupt(digitalPinToInterrupt(Alarm_InputPin), Alarm_Callback, FALLING);    // Enable interrupt from alarm
}

void RainGauge_Tipping_Callback(void) {
  RainGauge.Daily_Counter += 1;
}

void Alarm_Callback(void) {
  DS3231 alarm;
  bool alarm_1 = alarm.checkIfAlarm(1);   // Check and clear the alarm; otherwise, Alarm_InputPin would stay LOW.

  if (alarm_1) {
    unsigned long temp_Daily_Counter = RainGauge.Daily_Counter;   // Store Daily_Counter to a temporary variable to reset it
    RainGauge.Daily_Counter = 0;                                  // for the next daily count in case it's still counting (it's raining
                                                                  // at 9am when the alarm goes off).

    RainGauge.Rainfall_Data = RainGauge.RainfallPerTip * temp_Daily_Counter;    // Calculate the rainfall amount of the previous day

    RainGauge.RainGauge_DailyAlarm = true;
  }
}
