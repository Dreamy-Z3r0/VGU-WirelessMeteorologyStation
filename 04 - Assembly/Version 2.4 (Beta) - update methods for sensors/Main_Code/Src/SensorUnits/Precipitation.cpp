#include "../../Inc/SensorUnits/Precipitation.h"


/*********************
 *** Contructor(s) ***
 *********************/

// Empty constructor; Should never be called
Precipitation::Precipitation(void) {
  // Do nothing
}

// Accept sensor and alarm inputs; Set default values for class variables/settings
Precipitation::Precipitation(uint32_t SensorPin) {
  set_SensorPin(SensorPin);

  Daily_Counter = 0;
  Rainfall_Data = 0;

  clear_readFlag();
  set_standbyFlag();
}


/*****************************
 *** Device initialisation ***
 *****************************/

// Initial setups for communicating with and managing rain gauge
void Precipitation::init(void) {    
  pinMode(get_SensorPin(), INPUT_PULLUP);   // Initialize rain gauge input
  attachInterrupt(digitalPinToInterrupt(get_SensorPin()), std::bind(RainGauge_Tipping_Callback, this), FALLING);   // Routine on a bucket tip
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


/*******************************************
 *** External interrupt service routines ***
 *******************************************/

// Interrupt service routine when the bucket tips
void RainGauge_Tipping_Callback(Precipitation* PrecipitationInstance) {   
  PrecipitationInstance->Increment_Counter();
}