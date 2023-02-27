#include "Precipitation.h"


/*********************
 *** Contructor(s) ***
 *********************/

// Set initial values for variables (and class constants)
Precipitation::Precipitation(byte hour, byte minute, byte second) {
  Daily_Counter = 0;
  Rainfall_Data = 0;
  RainGauge_DailyAlarm = false;

  DailyAlarm_Time[0] = hour;
  DailyAlarm_Time[1] = minute;
  DailyAlarm_Time[2] = second;
}


/*****************************
 *** Device initialization ***
 *****************************/

// Initial setups for communicating with and managing rain gauge
void Precipitation::init(uint32_t RainGauge_InputPin, uint32_t Alarm_InputPin) {    
  pinMode(RainGauge_InputPin, INPUT_PULLUP);  // Initialize rain gauge input
  pinMode(Alarm_InputPin, INPUT_PULLUP);      // Initialize alarm pin

  attachInterrupt(digitalPinToInterrupt(RainGauge_InputPin), std::bind(RainGauge_Tipping_Callback, this), FALLING);   // Routine on a bucket tip

  DS3231 clock;
  clock.setA1Time(1, DailyAlarm_Time[0], DailyAlarm_Time[1], DailyAlarm_Time[2], 0x8, false, false, false);    // Set up daily alarm
  clock.turnOnAlarm(1);     // Enable alarm 1
  clock.checkIfAlarm(1);    // Make sure there is no alarm present initially

  attachInterrupt(digitalPinToInterrupt(Alarm_InputPin), std::bind(Alarm_Callback, this), LOW);    // Enable interrupt from alarm
}

// Set daily alarm (Default: 9 A.M)
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


/***********************************
 *** Data-returning operation(s) ***
 ***********************************/

// Return the latest calculated rainfall amount
float Precipitation::get_Rainfall_Data(void) {   // Return the latest calculated rainfall amount
  return Rainfall_Data;
}


/********************************
 *** Internal data processing ***
 ********************************/

// Update the rainfall amount of the previous day
void Precipitation::Update_Rainfall_Data(void) {    // Update the rainfall amount of the previous day
  unsigned long temp_Daily_Counter = Daily_Counter;   // Store Daily_Counter to a temporary variable to reset the count variable
  Daily_Counter = 0;                                  // for the next daily count in case it's still counting (it's raining
                                                      // when the alarm goes off).

  Rainfall_Data = RainfallPerTip * temp_Daily_Counter;    // Calculate the rainfall amount of the previous day
}

// Increment internal count whenever a bucket tip appears
void Precipitation::Increment_Counter(void) {   
  Daily_Counter += 1;
}


/************************
 *** Alarm processing ***
 ************************/

// When there's an alarm for a new day, clear it then request for an update of rainfall
void Precipitation::Alarm_Presence_Processing(void) {   
  DS3231 alarm;
  if (alarm.checkIfAlarm(1)) {    // Check and clear the alarm of Alarm 1 from DS3231; otherwise, Alarm_InputPin would stay LOW.
    Update_Rainfall_Data();
    RainGauge_DailyAlarm = true;
  }
}

// When there's an alarm for a new day, return TRUE
bool Precipitation::is_DailyAlarm_available(void) {   
  return RainGauge_DailyAlarm;
}

// Clear daily alarm flag after it is used and no longer necessary
void Precipitation::clear_DailyAlarm(void) {    
  RainGauge_DailyAlarm = false;
}


/*******************************************
 *** External interrupt service routines ***
 *******************************************/

// Interrupt service routine when the bucket tips
void RainGauge_Tipping_Callback(Precipitation* PrecipitationInstance) {   
  PrecipitationInstance->Increment_Counter();
}

// Interrupt service routine when alarm for a new day appears
void Alarm_Callback(Precipitation* PrecipitationInstance) {
  PrecipitationInstance->Alarm_Presence_Processing();
}
