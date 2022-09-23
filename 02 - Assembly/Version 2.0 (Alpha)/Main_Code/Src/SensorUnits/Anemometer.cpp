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
}


/*****************************
 *** Device initialization ***
 *****************************/

// Timer initialisation
void Anemometer_Control::init(TIM_TypeDef* AnemometerTimer_Instance) {
  /* Set initial value for class variables */
  update_standby(timeBetweenTwoReadingRoutines);

  set_standbyFlag();
  clear_readFlag();

  isTakingFirstEdge = false;
  isSecondEdgeDetected = false;

  CalmAir = false;
  CalmAirTiming_MaxValue = (uint8_t)(maxInput_windSpeed / (10E-3));

  Timing_Overflow = 0;

  arr_index = 0;
  fault_count = 0;

  /* Initiate timers */
  AnemometerTimer = new HardwareTimer(AnemometerTimer_Instance);
  
  AnemometerTimer->setMode(1, TIMER_DISABLED);    // Use channel 1 of AnemometerTimer_Instance in output compare mode, no output
  AnemometerTimer->setOverflow(10000, MICROSEC_FORMAT);   // Timer overflows every 10ms
  AnemometerTimer->attachInterrupt(1, std::bind(Anemometer_Control_TIM_Ovf_Callback, this));  // ISR run whenever timer overflows for channel 1

  clear_newDataReady();

  set_readFlag();
}


/**************************
 *** Reading operations ***
 **************************/

// Continuously called to perform wind speed reading
void Anemometer_Control::update_sensor_data(void) {
  if (is_readFlag_set() && is_standbyFlag_set()) {   // A new reading routine begins
    clear_standbyFlag();
    arr_index = 0;
    fault_count = 0;

    Initialise_New_Timing_Period();
  }
}

// Initiate a timing period for each data point of windSpeed[]
void Anemometer_Control::Initialise_New_Timing_Period(void) {
  isTakingFirstEdge = true;
  isSecondEdgeDetected = false;
  CalmAir = false;

  AnemometerTimer->pause();  // Make sure the timer is not running, then reset counter register
  AnemometerTimer->setCount(0);

  attachInterrupt(digitalPinToInterrupt(get_SensorPin()), std::bind(anemometerInput_Detected, this), RISING);
  AnemometerTimer->resume();
}

// Update windSpeed[] at the end of a timing period
void Anemometer_Control::WindSpeed_Array_Update_Routine(void) {
  if (CalmAir) {
    windSpeed[arr_index] = 0;
  }
  else {
    float measured_input_frequency;
    measured_input_frequency = 1 / ((AnemometerTimer->getCount(MICROSEC_FORMAT) * 1E-6) + (Timing_Overflow * 10E-3));
          
    windSpeed[arr_index] = measured_input_frequency * 2.4;
    if (maxKnown_windSpeed < windSpeed[arr_index]) {
      windSpeed[arr_index] = 0;
      fault_count += 1;
    }
  }

  arr_index += 1;
  if (dataPointsPerMeasurement == arr_index) {
    WindSpeed_MeanValue_Update_Routine();
  }
  else {
    Initialise_New_Timing_Period();
  }
}

// Calculate meanWindSpeed at the end of a reading period (arr_index reaches max value)
void Anemometer_Control::WindSpeed_MeanValue_Update_Routine(void) {
  if (fault_count < dataPointsPerMeasurement) {
    // Calculate mean wind speed
    float meanWindSpeed_temp = 0;
    uint8_t denominator = dataPointsPerMeasurement - fault_count;
    for (arr_index = 0; arr_index < dataPointsPerMeasurement; arr_index += 1) {
      meanWindSpeed_temp += (windSpeed[arr_index] / denominator);
    }
    meanWindSpeed = meanWindSpeed_temp;

    set_newDataReady();

    // Update new wind speed timestamp
    update_timestamp();
  }

  // End of the reading routine
  clear_readFlag();
  set_standbyFlag();
}


/***********************************
 *** Data-returning operation(s) ***
 ***********************************/

// Returns the latest wind speed value
void Anemometer_Control::read_sensor_data(float *external_storage) {    // Returns the latest wind direction value
  *external_storage = meanWindSpeed;
}



/*******************************************************************
 *** Internal operations for external interrupt service routines ***
 *******************************************************************/

// Timer counter overflow callback
void Anemometer_Control::Timer_Callback(void) {
  Timing_Overflow += 1;

  if (CalmAirTiming_MaxValue <= Timing_Overflow) {
    detachInterrupt(digitalPinToInterrupt(get_SensorPin()));   // Stop taking inputs

    AnemometerTimer->pause();         // Pause (running) timer
    AnemometerTimer->setCount(0);     // Reset timer counter
      
    Timing_Overflow = 0;          // Reset overflow counters

    isTakingFirstEdge = false;
    isSecondEdgeDetected = true;
    CalmAir = true;

    WindSpeed_Array_Update_Routine();
  }
}

// Edge presence callback
void Anemometer_Control::Input_Callback(void) {
  if (!isTakingFirstEdge) {
    AnemometerTimer->pause();   // Pause the running Anemometer timer
    
    detachInterrupt(digitalPinToInterrupt(get_SensorPin()));   // Stop taking inputs
    isSecondEdgeDetected = true;      // 2 edges timed
    
    CalmAir = false;    // Fail-safe only

    WindSpeed_Array_Update_Routine();
  }
  else {   
    AnemometerTimer->setCount(0);    // Reset CalmAir timer
    Timing_Overflow = 0;

    isTakingFirstEdge = false;    // Prepare for the next edge detection
  }
}


/*******************************************
 *** External interrupt service routines ***
 *******************************************/

// Interrupt service routine when a timer counter overflows
void Anemometer_Control_TIM_Ovf_Callback(Anemometer_Control* Anemometer_Instance) {
  Anemometer_Instance->Timer_Callback();
}

// Interrupt service routine when an edge is present at the input pin
void anemometerInput_Detected(Anemometer_Control* Anemometer_Instance) {
  Anemometer_Instance->Input_Callback();
}
