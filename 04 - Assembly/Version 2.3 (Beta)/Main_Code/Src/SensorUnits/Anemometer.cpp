#include "../../Inc/SensorUnits/Anemometer.h"


/*********************
 *** Contructor(s) ***
 *********************/

// Class constructor without input parameter(s)
Anemometer_Control::Anemometer_Control(void) {
  // Do nothing inside this constructor
}

// Accept input pin parameter
Anemometer_Control::Anemometer_Control(uint32_t SensorPin) {    
  set_SensorPin(SensorPin);

  clear_readFlag();
}


/*****************************
 *** Device initialisation ***
 *****************************/

void Anemometer_Control::init(void) {
  // Initial value for count
  count = 0;

  // Attach an interrupt on anemometer input pin for falling edge detection
  attachInterrupt(digitalPinToInterrupt(get_SensorPin()), std::bind(anemometerInput_Detected, this), FALLING);
}


/***********************************
 *** Data-returning operation(s) ***
 ***********************************/

// Returns the latest wind speed value
void Anemometer_Control::read_sensor_data(float *external_storage) {    // Returns the latest wind direction value
  if (is_readFlag_set()) {
    // Clear read request
    clear_readFlag();

    // Save then reset the count variable 
    int count_temp = count;
    count = 0;

    // Calculate and return the wind speed from the count variable
    meanWindSpeed = wind_speed_conversion(count_temp);
    *external_storage = meanWindSpeed;
  }
}

// Calculate wind speed from recorded number of edges
float Anemometer_Control::wind_speed_conversion(int pulseQuantity) {
  /* Method 1: Using datasheet constant 
  "A wind speed of 1.492 MPH (2.4 km/h) causes the switch to close once per second."
  For an observation period of n minutes, the switch closes n*60 times by the 2.4-km/h wind, thus n*60 edges.
  As a result, if m pulses are detected over n minutes, the wind speed is then 2.4*(m/(n*60)) km/h, or m/(n*90) m/s.
  */

  return ((1.0 * pulseQuantity) / (UpdateInterval / 90.0));   // m/s


  /* Method 2: Using anemometer dimensions 
  Since the switch closes twice per revolution, if m pulses are detected, the anemometer has made m/2 rotations.
  The average wind speed (in m/s) is then (m/2) * (2*pi*R/T), supposedly; 
  with R being anemometer radius in metres and T being the observation period in seconds.
  With the anemometer factor k taken into account, the wind speed is k * (m/2) * (2*pi*R/T).
  */
 
  // return AnemometerFactor * ((pulseQuantity / 2.0) * ((TWO_PI * AnemometerRadius) / (UpdateInterval * 60.0)));   // m/s
}


/*********************************************************************
 *** Internal operations for external interrupt service routine(s) ***
 *********************************************************************/

// Edge presence callback
void Anemometer_Control::Input_Callback(void) {
  // Increment the count variable
  count += 1;
}


/*********************************************
 *** External interrupt service routine(s) ***
 *********************************************/

// Interrupt service routine when an edge is present at the input pin
void anemometerInput_Detected(Anemometer_Control* Anemometer_Instance) {
  Anemometer_Instance->Input_Callback();
}
