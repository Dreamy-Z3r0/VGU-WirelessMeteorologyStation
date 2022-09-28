#include "../../Inc/SensorUnits/Precipitation.h"


/*********************
 *** Contructor(s) ***
 *********************/

// Empty constructor; Should never be called
Precipitation::Precipitation(void) {
  // Do nothing
}

// Accept sensor and alarm inputs; Set default values for class variables/settings
Precipitation::Precipitation(uint32_t SensorPin, uint32_t Alarm_InputPin) {
  set_SensorPin(SensorPin);
  this->Alarm_InputPin = Alarm_InputPin;

  Daily_Counter = 0;
  Rainfall_Data = 0;
  RainGauge_DailyAlarm = false;

  DailyAlarm_Time[0] = 9;
  DailyAlarm_Time[1] = 0;
  DailyAlarm_Time[2] = 0;
}

// Accept sensor and alarm inputs; (Optional) accept user-input alarm settings
Precipitation::Precipitation(uint32_t SensorPin, uint32_t Alarm_InputPin, byte hour, byte minute, byte second) {
  set_SensorPin(SensorPin);
  this->Alarm_InputPin = Alarm_InputPin;
  
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
void Precipitation::init(void) {    
  pinMode(get_SensorPin(), INPUT_PULLUP);   // Initialize rain gauge input
  pinMode(Alarm_InputPin, INPUT_PULLUP);    // Initialize alarm pin

  attachInterrupt(digitalPinToInterrupt(get_SensorPin()), std::bind(RainGauge_Tipping_Callback, this), FALLING);   // Routine on a bucket tip

  DS3231 clock;
  clock.setA1Time(1, DailyAlarm_Time[0], DailyAlarm_Time[1], DailyAlarm_Time[2], 0x8, false, false, false);    // Set up daily alarm
  clock.turnOnAlarm(1);     // Enable alarm 1
  clock.checkIfAlarm(1);    // Make sure there is no alarm present initially

  attachInterrupt(digitalPinToInterrupt(Alarm_InputPin), std::bind(Alarm_Callback, this), LOW);    // Enable interrupt from alarm

  clear_newDataReady();
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
void Precipitation::read_sensor_data(float *external_storage) {   // Return the latest calculated rainfall amount
  *external_storage = Rainfall_Data;
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

  // Update timestamp
  update_timestamp();

  set_newDataReady();
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
