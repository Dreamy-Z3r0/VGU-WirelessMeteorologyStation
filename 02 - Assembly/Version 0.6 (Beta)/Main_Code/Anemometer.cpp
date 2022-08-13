#include "Anemometer.h"


Anemometer_Control::Anemometer_Control(uint32_t Input_Pin) {
  this->Input_Pin = Input_Pin;

  idleTimeInMilliseconds = timeBetweenReadingPeriods * 60E3;

  idlePeriod = true;
  readFlag = false;

  isTakingFirstEdge = false;
  isSecondEdgeDetected = false;

  CalmAir = false;
  CalmAirTiming_MaxValue = (uint8_t)(maxInput_windSpeed / (10E-3));

  EdgeTiming_Overflow = 0;
  CalmAirTiming_Overflow = 0;

  arr_index = 0;
  fault_count = 0;
}

void Anemometer_Control::Init(TIM_TypeDef* EdgePeriodTimer_Instance, TIM_TypeDef* CalmAirTimer_Instance) {
  EdgePeriodTimer = new HardwareTimer(EdgePeriodTimer_Instance);
  CalmAirTimer = new HardwareTimer(CalmAirTimer_Instance);
  
  EdgePeriodTimer->setMode(1, TIMER_DISABLED);    // Use channel 1 of EdgePeriodTimer_Instance in output compare mode, no output
  EdgePeriodTimer->setOverflow(10000, MICROSEC_FORMAT);   // Timer overflows every 10ms
  EdgePeriodTimer->attachInterrupt(1, std::bind(TIM_Ovf_callback, this, EdgePeriodTimer));  // ISR run whenever timer overflows for channel 1

  CalmAirTimer->setMode(2, TIMER_DISABLED);    // Use channel 1 of CalmAirTimer_Instance in output compare mode, no output
  CalmAirTimer->setOverflow(10000, MICROSEC_FORMAT);      // Timer overflows every 10ms
  CalmAirTimer->attachInterrupt(2, std::bind(TIM_Ovf_callback, this, CalmAirTimer));  // ISR run whenever timer overflows for channel 1

  readFlag = true;
}

void Anemometer_Control::Initialise_New_Timing_Period(void) {
  isTakingFirstEdge = true;
  isSecondEdgeDetected = false;
  CalmAir = false;

  EdgePeriodTimer->pause();  // Make sure the timer is not running, then reset counter register
  EdgePeriodTimer->setCount(0);

  CalmAirTimer->pause();    // Make sure the timer is not running, then reset counter register
  CalmAirTimer->setCount(0);

  attachInterrupt(digitalPinToInterrupt(Input_Pin), std::bind(anemometerInput_Detected, this), RISING);
  CalmAirTimer->resume();
}

void Anemometer_Control::Anemometer_Reading_Routine(void) {
  if (readFlag) {
    if (idlePeriod) {   // A new reading routine begins
      idlePeriod = false;
      fault_count = 0;

      Initialise_New_Timing_Period();
    }
    else {
      if ((!isTakingFirstEdge) && isSecondEdgeDetected) {
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
          if (fault_count < dataPointsPerMeasurement) {
            // Calculate mean wind speed
            meanWindSpeed = 0;
            uint8_t denominator = dataPointsPerMeasurement - fault_count;
            for (arr_index = 0; arr_index < dataPointsPerMeasurement; arr_index += 1) {
              meanWindSpeed += (windSpeed[arr_index] / denominator);
            }

            // Update new wind speed timestamp
            // -> do it here <-
          }

          // End of the reading routine
          readFlag = false;
          idlePeriod = true;
        }
        else {
          Initialise_New_Timing_Period();
        }
      }
    }
  }
}

void Anemometer_Control::Timer_Callback(HardwareTimer* OverflownTimer) {
  if (EdgePeriodTimer == OverflownTimer) {
    EdgeTiming_Overflow += 1;
  }
  else if (CalmAirTimer == OverflownTimer) {
    CalmAirTiming_Overflow += 1;

    if (CalmAirTiming_MaxValue <= CalmAirTiming_Overflow) {
      EdgePeriodTimer->pause();         // Pause (running) timers
      CalmAirTimer->pause();

      detachInterrupt(digitalPinToInterrupt(Input_Pin));   // Stop taking inputs

      EdgePeriodTimer->setCount(0);     // Reset timer counters
      CalmAirTimer->setCount(0);
      
      EdgeTiming_Overflow = 0;          // Reset overflow counters
      CalmAirTiming_Overflow = 0;

      isTakingFirstEdge = false;
      isSecondEdgeDetected = true;
      CalmAir = true;
    }
  }
}

void Anemometer_Control::Input_Callback(void) {
  CalmAirTimer->pause();    // Pause the running CalmAir timer
  
  if (!isTakingFirstEdge) {
    EdgePeriodTimer->pause();   // Pause running EdgePeriod timer
    
    detachInterrupt(digitalPinToInterrupt(Input_Pin));   // Stop taking inputs
    isSecondEdgeDetected = true;      // 2 edges timed
    
    CalmAir = false;    // Fail-safe only
  }
  else {   
    CalmAirTimer->setCount(0);    // Reset CalmAir timer
    CalmAirTimer->resume();
    
    EdgePeriodTimer->resume();    // Resume EdgePeriod timer

    isTakingFirstEdge = false;    // Prepare for the next edge detection
  }
}


void TIM_Ovf_callback(Anemometer_Control* Anemometer_Instance, HardwareTimer* OverflownTimer) {
  Anemometer_Instance->Timer_Callback(OverflownTimer);
}

void anemometerInput_Detected(Anemometer_Control* Anemometer_Instance) {
  Anemometer_Instance->Input_Callback();
}
