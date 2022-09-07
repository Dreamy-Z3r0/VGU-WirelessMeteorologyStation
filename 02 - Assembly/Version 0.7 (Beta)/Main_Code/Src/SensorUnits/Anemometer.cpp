#include "../../Inc/SensorUnits/Anemometer.h"


/*********************
 *** Contructor(s) ***
 *********************/

// Accept input pin parameter and set (initial) values of variables and class constants
Anemometer_Control::Anemometer_Control(uint32_t SensorPin) {    
  set_SensorPin(SensorPin);

  idleTimeInMilliseconds = timeBetweenReadingPeriods * 60E3;

  set_standbyFlag();
  clear_readFlag();

  isTakingFirstEdge = false;
  isSecondEdgeDetected = false;

  CalmAir = false;
  CalmAirTiming_MaxValue = (uint8_t)(maxInput_windSpeed / (10E-3));

  EdgeTiming_Overflow = 0;
  CalmAirTiming_Overflow = 0;

  arr_index = 0;
  fault_count = 0;
}


/*****************************
 *** Device initialization ***
 *****************************/

// Timer initialisation
void Anemometer_Control::init(TIM_TypeDef* EdgePeriodTimer_Instance, TIM_TypeDef* CalmAirTimer_Instance) {
  EdgePeriodTimer = new HardwareTimer(EdgePeriodTimer_Instance);
  CalmAirTimer = new HardwareTimer(CalmAirTimer_Instance);
  
  EdgePeriodTimer->setMode(1, TIMER_DISABLED);    // Use channel 1 of EdgePeriodTimer_Instance in output compare mode, no output
  EdgePeriodTimer->setOverflow(10000, MICROSEC_FORMAT);   // Timer overflows every 10ms
  EdgePeriodTimer->attachInterrupt(1, std::bind(TIM_Ovf_callback, this, EdgePeriodTimer));  // ISR run whenever timer overflows for channel 1

  CalmAirTimer->setMode(2, TIMER_DISABLED);    // Use channel 1 of CalmAirTimer_Instance in output compare mode, no output
  CalmAirTimer->setOverflow(10000, MICROSEC_FORMAT);      // Timer overflows every 10ms
  CalmAirTimer->attachInterrupt(2, std::bind(TIM_Ovf_callback, this, CalmAirTimer));  // ISR run whenever timer overflows for channel 1

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

  EdgePeriodTimer->pause();  // Make sure the timer is not running, then reset counter register
  EdgePeriodTimer->setCount(0);

  CalmAirTimer->pause();    // Make sure the timer is not running, then reset counter register
  CalmAirTimer->setCount(0);

  attachInterrupt(digitalPinToInterrupt(get_SensorPin()), std::bind(anemometerInput_Detected, this), RISING);
  CalmAirTimer->resume();
}

// Update windSpeed[] at the end of a timing period
void Anemometer_Control::WindSpeed_Array_Update_Routine(void) {
  if (CalmAir) {
    windSpeed[arr_index] = 0;
  }
  else {
    float measured_input_frequency;
    measured_input_frequency = 1 / ((EdgePeriodTimer->getCount(MICROSEC_FORMAT) * 1E-6) + (EdgeTiming_Overflow * 10E-3));
          
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
void Anemometer_Control::Timer_Callback(HardwareTimer* OverflownTimer) {
  if (EdgePeriodTimer == OverflownTimer) {
    EdgeTiming_Overflow += 1;
  }
  else if (CalmAirTimer == OverflownTimer) {
    CalmAirTiming_Overflow += 1;

    if (CalmAirTiming_MaxValue <= CalmAirTiming_Overflow) {
      EdgePeriodTimer->pause();         // Pause (running) timers
      CalmAirTimer->pause();

      detachInterrupt(digitalPinToInterrupt(get_SensorPin()));   // Stop taking inputs

      EdgePeriodTimer->setCount(0);     // Reset timer counters
      CalmAirTimer->setCount(0);
      
      EdgeTiming_Overflow = 0;          // Reset overflow counters
      CalmAirTiming_Overflow = 0;

      isTakingFirstEdge = false;
      isSecondEdgeDetected = true;
      CalmAir = true;

      WindSpeed_Array_Update_Routine();
    }
  }
}

// Edge presence callback
void Anemometer_Control::Input_Callback(void) {
  CalmAirTimer->pause();    // Pause the running CalmAir timer
  
  if (!isTakingFirstEdge) {
    EdgePeriodTimer->pause();   // Pause running EdgePeriod timer
    
    detachInterrupt(digitalPinToInterrupt(get_SensorPin()));   // Stop taking inputs
    isSecondEdgeDetected = true;      // 2 edges timed
    
    CalmAir = false;    // Fail-safe only

    WindSpeed_Array_Update_Routine();
  }
  else {   
    CalmAirTimer->setCount(0);    // Reset CalmAir timer
    CalmAirTimer->resume();
    
    EdgePeriodTimer->resume();    // Resume EdgePeriod timer

    isTakingFirstEdge = false;    // Prepare for the next edge detection
  }
}


/*******************************************
 *** External interrupt service routines ***
 *******************************************/

// Interrupt service routine when a timer counter overflows
void TIM_Ovf_callback(Anemometer_Control* Anemometer_Instance, HardwareTimer* OverflownTimer) {
  Anemometer_Instance->Timer_Callback(OverflownTimer);
}

// Interrupt service routine when an edge is present at the input pin
void anemometerInput_Detected(Anemometer_Control* Anemometer_Instance) {
  Anemometer_Instance->Input_Callback();
}
