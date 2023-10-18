#include "../../Inc/SensorUnits/Anemometer.h"


const float numerator[] = {1.698, 3.797, 2.632};
const float denominator[] = {1, -0.002605, 0.1079};


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

  // clear_readFlag();
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


/**************************
 *** Reading operations ***
 **************************/

void Anemometer_Control::update_sensor_data(void) {
  if (is_readFlag_set() && is_standbyFlag_set()) {
    // Clear standby flag and enter data update routine
    clear_standbyFlag();

    // Save, then reset the count variable 
    int count_temp = count;
    count = 0;

    // Calculate and return the wind speed from the count variable
    meanWindSpeed = wind_speed_conversion(count_temp);

    // Set standby flag value at the end of an data update routine
    set_standbyFlag();
  }
}


/***********************************
 *** Data-returning operation(s) ***
 ***********************************/

// Returns the latest wind speed value
void Anemometer_Control::read_sensor_data(float *external_storage) {    // Returns the latest wind direction value
  if (is_readFlag_set() && is_standbyFlag_set()) {    // Double-check status flags to avoid error(s)
    *external_storage = meanWindSpeed;      // Return wind speed data
    clear_readFlag();    // End of reading routine
  }
}

// Calculate wind speed from recorded number of edges
float Anemometer_Control::wind_speed_conversion(int pulseQuantity) {
  /* Method 1: Using datasheet constant   <- depicted
  "A wind speed of 1.492 MPH (2.4 km/h) causes the switch to close once per second."
  For an observation period of n minutes, the switch closes n*60 times by the 2.4-km/h wind, thus n*60 edges.
  As a result, if m pulses are detected over n minutes, the wind speed is then 2.4*(m/(n*60)) km/h, or m/(n*90) m/s.
  */

  // return ((1.0 * pulseQuantity) / (UpdateInterval / 90.0));   // m/s


  /* Method 2: Using anemometer dimensions 
  Since the switch closes twice per revolution, if m pulses are detected, the anemometer has made m/2 rotations.
  The average wind speed (in m/s) is then (m/2) * (2*pi*R/T), supposedly; 
  with R being anemometer radius in metres and T being the observation period in seconds.
  With the anemometer factor k taken into account, the wind speed is k * (m/2) * (2*pi*R/T).
  */
 
  float readWindSpeed = ((pulseQuantity / 2.0) * ((TWO_PI * AnemometerRadius) / (WindDataUpdateInterval * 60.0)));   // m/s
  return AnemometerFactor(readWindSpeed) * readWindSpeed;   // m/s
}

float Anemometer_Control::AnemometerFactor(float readWindSpeed) {
  #if defined (ConstantAnemometerFactor)
  return ConstantAnemometerFactor
  #else
  int numerator_degree = (sizeof(numerator) / sizeof(numerator[0])) - 1;
  int denominator_degree = (sizeof(denominator) / sizeof(denominator[0])) - 1;

  float curve_numerator = 0, curve_denominator = 0;

  for (int exponent = numerator_degree; exponent >= 0; exponent -= 1) {
    curve_numerator += numerator[numerator_degree-exponent] * pow(readWindSpeed, exponent);
  }

  for (int exponent = denominator_degree; exponent >= 0; exponent -= 1) {
    curve_denominator += denominator[denominator_degree-exponent] * pow(readWindSpeed, exponent);
  }

  return (curve_numerator/curve_denominator);
  #endif
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
