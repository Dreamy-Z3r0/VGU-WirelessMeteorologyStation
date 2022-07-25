#include "Precipitation.h"


/***********************************
 *** Precipitation class methods ***
 ***********************************/

 /* Constructor */
Precipitation::Precipitation(byte hour, byte minute, byte second) {
  Daily_Counter = 0;
  Rainfall_Data = 0;
  RainGauge_DailyAlarm = false;

  DailyAlarm_Time[0] = hour;
  DailyAlarm_Time[1] = minute;
  DailyAlarm_Time[2] = second;
}

 /* Initialisation */
void Precipitation::init(uint32_t RainGauge_InputPin, uint32_t Alarm_InputPin) {    // Initial set-ups for communicating with and managing rain gauge
  pinMode(RainGauge_InputPin, INPUT_PULLUP);  // Initialize rain gauge input
  pinMode(Alarm_InputPin, INPUT_PULLUP);      // Initialize alarm pin

  attachInterrupt(digitalPinToInterrupt(RainGauge_InputPin), std::bind(RainGauge_Tipping_Callback, this), FALLING);   // Routine on a bucket tip

  DS3231 clock;
  clock.setA1Time(1, DailyAlarm_Time[0], DailyAlarm_Time[1], DailyAlarm_Time[2], 0x8, false, false, false);    // Set up daily alarm
  clock.turnOnAlarm(1);     // Enable alarm 1
  clock.checkIfAlarm(1);    // Make sure there is no alarm present initially

  attachInterrupt(digitalPinToInterrupt(Alarm_InputPin), std::bind(Alarm_Callback, this), LOW);    // Enable interrupt from alarm
}

void Precipitation::set_DailyAlarm(byte hour, byte minute, byte second) {
  DailyAlarm_Time[0] = hour;
  DailyAlarm_Time[1] = minute;
  DailyAlarm_Time[2] = second;

  DS3231 clock;
  clock.turnOffAlarm(1);    // Disable alarm 1
  clock.setA1Time(1, DailyAlarm_Time[0], DailyAlarm_Time[1], DailyAlarm_Time[2], 0x8, false, false, false);    // Set up new daily alarm
  clock.turnOnAlarm(1);     // Enable alarm 1
  clock.checkIfAlarm(1);    // Make sure there is no alarm present initially
}

 /* Data-returning method */
double Precipitation::get_Rainfall_Data(void) {   // Return the latest calculated rainfall amount
  return Rainfall_Data;
}

 /* Internal data processing */
void Precipitation::Increment_Counter(void) {   // Increment internal count variable whenever a bucket tip appears
  Daily_Counter += 1;
}

void Precipitation::Update_Rainfall_Data(void) {    // Update the rainfall amount of the previous day
  unsigned long temp_Daily_Counter = Daily_Counter;   // Store Daily_Counter to a temporary variable to reset it
  Daily_Counter = 0;                                  // for the next daily count in case it's still counting (it's raining
                                                      // at 9am when the alarm goes off).

  Rainfall_Data = RainfallPerTip * temp_Daily_Counter;    // Calculate the rainfall amount of the previous day
}

 /* Alarm processing */
void Precipitation::Alarm_Presence_Processing(void) {   // When there's an alarm for a new day, clear it then request for an update of rainfall
  DS3231 alarm;
  if (alarm.checkIfAlarm(1)) {    // Check and clear the alarm of Alarm 1 from DS3231; otherwise, Alarm_InputPin would stay LOW.
    Update_Rainfall_Data();
    RainGauge_DailyAlarm = true;
  }
}

bool Precipitation::is_DailyAlarm_available(void) {   // When there's an alarm for a new day, return TRUE
  return RainGauge_DailyAlarm;
}

void Precipitation::clear_DailyAlarm(void) {    // Clear daily alarm flag after it is used and no longer necessary
  RainGauge_DailyAlarm = false;
}


/*************************************
 *** External supporting functions ***
 *************************************/
void RainGauge_Tipping_Callback(Precipitation* PrecipitationInstance) {   // Interrupt service routine when a bucket tip appears
  PrecipitationInstance->Increment_Counter();
}

void Alarm_Callback(Precipitation* PrecipitationInstance) {               // Interrupt service routine when alarm for a new day appears
  PrecipitationInstance->Alarm_Presence_Processing();
}
