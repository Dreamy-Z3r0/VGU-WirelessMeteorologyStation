#include "Anemometer.h"


Anemometer_Control::Anemometer_Control(uint32_t Input_Pin, TIM_TypeDef *Instance_TIM) {
  this->Input_Pin = Input_Pin;

  InputRoutineTim = new HardwareTimer(Instance_TIM);

  readFlag = false;
  ongoingMeasurement = false;
  endOfMeasurement = false;

  idlePeriod = true;
  calmAir = false;

  Sampling_Window_Tracker = 0;
}

void Anemometer_Control::Init(void) {
  InputRoutineTim->setMode(1, TIMER_DISABLED);    // Use channel 1 of TIM1 in output compare mode, no output
  // InputRoutineTim->setPrescaleFactor(72);      // Prescaler = 72 => TimClk = SysClk / 72 = 72MHz / 72 = 1MHz => Timer counter increments every 1us
                                                  // setPrescaleFactor() is not necessary since setOverflow() is called with MICROSEC_FORMAT, thus automatically updated timer clock
  InputRoutineTim->setOverflow(10000, MICROSEC_FORMAT);   // Timer overflows every 10ms
  InputRoutineTim->attachInterrupt(1, std::bind(TIM_Ovf_callback, this));  // ISR run whenever timer overflows for channel 1

  InputRoutineTim->pause();       // Make sure timer is not running and the count register is reset.
  InputRoutineTim->setCount(0);
}

float Anemometer_Control::get_Wind_Speed(void) {
  return windSpeed;
}

void Anemometer_Control::read_Wind_Speed(void) {
  idlePeriod = false;
  
  float windSpeed_array[dataPointsPerMeasurement];
  uint8_t arrayIndex = 0;

  windSpeed = -404;
  while (windSpeed == -404) {
    Calm_Air_Detection();
    Initiate_Data_Point_Read();
    End_Of_Sampling_Window_Routine(windSpeed_array, &arrayIndex);
  }

  readFlag = true;    // Just for consistency across all the classes
  idlePeriod = true;
}

void Anemometer_Control::Calm_Air_Detection(void) {
  if (Sampling_Window_Tracker > (maxInput_windSpeed * 1E3)) {
    InputRoutineTim->pause();         // Stop timer counter if it's running
    detachInterrupt(digitalPinToInterrupt(Input_Pin));   // Stop taking inputs

    readFlag = false;
    ongoingMeasurement = false;

    endOfMeasurement = true;
    calmAir = true;
  }
}

void Anemometer_Control::Anemometer_Input_Routine(void) {
  if (ongoingMeasurement) {
    InputRoutineTim->pause();         // Stop timer counter
    detachInterrupt(digitalPinToInterrupt(Input_Pin));   // Stop taking inputs
    ongoingMeasurement = false;   // 2 edges timed

    endOfMeasurement = true;
  }
  else {
    InputRoutineTim->resume();        // Start timer counter
    ongoingMeasurement = true;    // Prepare for the next edge detection
  }
}

void Anemometer_Control::Initiate_Data_Point_Read(void) {
  if (readFlag) {
    readFlag = false;   // Clear current request for a sampling window
    ongoingMeasurement = false;   // Prepare for the next edge detection

    InputRoutineTim->setCount(0);     // Reset timer counter
    overflow_counter = 0;

    attachInterrupt(digitalPinToInterrupt(Input_Pin), std::bind(anemometerInput_Detected, this), RISING);
  }
}

void Anemometer_Control::End_Of_Sampling_Window_Routine(float* windSpeed_array, uint8_t* arrayIndex) {
  if (endOfMeasurement) {
    endOfMeasurement = false;
    
    if (!calmAir) {
      float measured_input_frequency;
      measured_input_frequency = 1 / ((InputRoutineTim->getCount(MICROSEC_FORMAT) * 1E-6) + (overflow_counter * 10E-3));
      windSpeed_array[*arrayIndex] = measured_input_frequency * 2.4;
    }
    else {
      calmAir = false;
      windSpeed_array[*arrayIndex] = 0;
    }

    *arrayIndex += 1;
    if (dataPointsPerMeasurement == *arrayIndex) {
      windSpeed = 0;
      uint8_t faulty_readings = 0;

      for (*arrayIndex = 0; *arrayIndex < dataPointsPerMeasurement; *arrayIndex += 1) {
        if (maxKnown_windSpeed < windSpeed_array[*arrayIndex]) {
          faulty_readings += 1;
        }
        else {
          windSpeed += windSpeed_array[*arrayIndex];
        }
      }

      if (faulty_readings < dataPointsPerMeasurement) {
        windSpeed /= (dataPointsPerMeasurement - faulty_readings);
      }
      else {
        windSpeed = -1;
      }

      *arrayIndex = 0;
    }
    else {
      readFlag = true;
    }
  }
}

void Anemometer_Control::set_readFlag(void) {
  readFlag = true;
}

bool Anemometer_Control::is_readFlag_set(void) {
  return readFlag;
}

void Anemometer_Control::clear_readFlag(void) {   // -> unused
  readFlag = false;
}

bool Anemometer_Control::is_idle(void) {
  return idlePeriod;
}


void TIM_Ovf_callback(Anemometer_Control* Anemometer_Instance) {
  Anemometer_Instance->overflow_counter += 1;
}

void anemometerInput_Detected(Anemometer_Control* Anemometer_Instance) {
  Anemometer_Instance->Anemometer_Input_Routine();
}
