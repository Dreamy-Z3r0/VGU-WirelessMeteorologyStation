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

  clear_readFlag();
  set_standbyFlag();
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

  clear_readFlag();
  set_standbyFlag();
}


/*****************************
 *** Device initialisation ***
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


/****************************
 *** Reading operation(s) ***
 ****************************/

// Update the rainfall amount of the previous day
void Precipitation::update_sensor_data(void) {
  if (is_readFlag_set() && is_standbyFlag_set()) {
    clear_standbyFlag();
  
    // Store Daily_Counter to a temporary variable to reset the count variable
    // for the next daily count in case it's still counting (it's raining
    // when the alarm goes off).
    unsigned long temp_Daily_Counter = Daily_Counter;   
    Daily_Counter = 0;                                  

    // Calculate the rainfall amount of the previous day                                                
    Rainfall_Data = RainfallPerTip * temp_Daily_Counter;    

    // Set standby flag value at the end of an data update routine
    set_standbyFlag();
  }
}


/***********************************
 *** Data-returning operation(s) ***
 ***********************************/

// Return the latest calculated rainfall amount
void Precipitation::read_sensor_data(float *external_storage) {   // Return the latest calculated rainfall amount
  if (is_readFlag_set() && is_standbyFlag_set()) {    // Double-check status flags to avoid error(s)
    *external_storage = Rainfall_Data;    // Return precipitation data
    clear_readFlag();   // Clear read request
  }
}


/********************************
 *** Internal data processing ***
 ********************************/

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
    // Enable a read request for Sensor_Control
    set_readFlag();
  }
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
