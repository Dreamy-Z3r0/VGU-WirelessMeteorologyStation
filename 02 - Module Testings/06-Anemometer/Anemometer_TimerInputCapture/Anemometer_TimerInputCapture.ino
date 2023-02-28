#define KMPH_TO_MPS(KMPH) (KMPH / 3.6)
#define KMPH_TO_MPH(KMPH) (KMPH / 1.609344)

#define input_Anemometer PA3
#define maxInput_windSpeed 2.38   // seconds
#define maxKnown_windSpeed 407.16 // km/h
#define dataPointsPerMeasurement 10   // Number of data points per measurement to take the final mean value

#define timeBetweenReadingPeriods 0.5   // minutes
unsigned long idleTimeInMilliseconds;

unsigned long overflow_counter = 0;

TIM_TypeDef *Instance_TIM1 = TIM1;   
HardwareTimer *InputRoutineTim = new HardwareTimer(Instance_TIM1);

bool sensorReadingRequested = false,
     sensorReadingInitiated = false,
     sensorReadingFinished  = false;

float measured_input_frequency, windSpeed[dataPointsPerMeasurement];
unsigned long start_timestamp, stop_timestamp, samplingDuration[dataPointsPerMeasurement];

uint8_t arrayIndex = 0;
float mean_windSpeed;
unsigned long mean_samplingDuration;

bool idlePeriod = false;
bool calmAir = false;
unsigned long timestamp;

void setup() {
  Serial.begin(9600);
  pinMode(input_Anemometer, INPUT);

  idleTimeInMilliseconds = timeBetweenReadingPeriods * 60E3;

  InputRoutineTim->setMode(1, TIMER_DISABLED);    // Use channel 1 of TIM1 in output compare mode, no output
  // InputRoutineTim->setPrescaleFactor(72);      // Prescaler = 72 => TimClk = SysClk / 72 = 72MHz / 72 = 1MHz => Timer counter increments every 1us
                                                  // setPrescaleFactor() is not necessary since setOverflow() is called with MICROSEC_FORMAT, thus automatically updated timer clock
  InputRoutineTim->setOverflow(10000, MICROSEC_FORMAT);   // Timer overflows every 10ms
  InputRoutineTim->attachInterrupt(1, TIM_Ovf_callback);  // ISR run whenever timer overflows for channel 1

  sensorReadingRequested = true;
}

void loop() {
  if (idlePeriod) {
    if (timestamp > millis()) {   // request a new wind speed update in case timer for millis() overflows
      idlePeriod = false;
      sensorReadingRequested = true;
      timestamp = millis();
    }
    else if ((millis() - timestamp) >= idleTimeInMilliseconds) {  // request a new wind speed update after [timeBetweenReadingPeriods]-minute idle period
      idlePeriod = false;
      sensorReadingRequested = true;
      timestamp = millis();
    }
  }
  else {
    if ((millis() - timestamp) > (maxInput_windSpeed * 1E3)) {
      InputRoutineTim->pause();         // Stop timer counter if it's running
      detachInterrupt(digitalPinToInterrupt(input_Anemometer));   // Stop taking inputs
      
      sensorReadingRequested = false;
      sensorReadingInitiated = false;

      sensorReadingFinished = true;
      calmAir = true;
    }
  }
  
  if (sensorReadingRequested) {
    sensorReadingRequested = false;   // Clear request
    sensorReadingInitiated = false;   // Prepare for the next edge detection
    
    InputRoutineTim->setCount(0);     // Reset timer counter
    overflow_counter = 0;

    start_timestamp = micros();
    attachInterrupt(digitalPinToInterrupt(input_Anemometer), anemometerInput_Detected, RISING);
  }

  if (sensorReadingFinished) {
    sensorReadingFinished = false;
    stop_timestamp = micros();

    if (!calmAir) {
      measured_input_frequency = 1 / ((InputRoutineTim->getCount(MICROSEC_FORMAT) * 1E-6) + (overflow_counter * 10E-3));
      windSpeed[arrayIndex] = measured_input_frequency * 2.4;
    }
    else {
      calmAir = false;
      windSpeed[arrayIndex] = 0;
    }

    samplingDuration[arrayIndex] = stop_timestamp - start_timestamp;
    arrayIndex += 1;

    if (dataPointsPerMeasurement == arrayIndex) {
      mean_windSpeed = 0;
      mean_samplingDuration = 0;

      uint8_t faulty_readings = 0;
      
      for (arrayIndex = 0; arrayIndex < dataPointsPerMeasurement; arrayIndex += 1) {
        if (maxKnown_windSpeed < windSpeed[arrayIndex]) {
          faulty_readings += 1;
        }
        else {
          mean_windSpeed += windSpeed[arrayIndex];
          mean_samplingDuration += samplingDuration[arrayIndex];
        }
      }

      if (faulty_readings < dataPointsPerMeasurement) {
        mean_windSpeed /= (dataPointsPerMeasurement - faulty_readings);
        mean_samplingDuration /= (dataPointsPerMeasurement - faulty_readings);

        Serial.print("Wind speed: ");

        Serial.print(KMPH_TO_MPS(mean_windSpeed), 2);
        Serial.print(" m/s - ");

        Serial.print(mean_windSpeed, 2);
        Serial.print(" km/h - ");

        Serial.print(KMPH_TO_MPH(mean_windSpeed), 2);
        Serial.println(" mph");

        Serial.print("Read time: ");
        Serial.print(mean_samplingDuration/1000.0, 2);
        Serial.println(" ms\n");
      }
      
      arrayIndex = 0;
      idlePeriod = true;
    }
    else {
      sensorReadingRequested = true;
    }

    timestamp = millis();
  }
}

void TIM_Ovf_callback(void) {
  overflow_counter += 1;
}

void anemometerInput_Detected(void) {
  if (sensorReadingInitiated) {
    InputRoutineTim->pause();         // Stop timer counter
    detachInterrupt(digitalPinToInterrupt(input_Anemometer));   // Stop taking inputs
    sensorReadingInitiated = false;   // 2 edges timed

    sensorReadingFinished = true;
  }
  else {
    InputRoutineTim->resume();        // Start timer counter
    sensorReadingInitiated = true;    // Prepare for the next edge detection
  }
}
