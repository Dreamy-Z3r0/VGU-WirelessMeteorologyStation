#define NOP __asm__ __volatile__ ("nop\n\t")

#define KMPH_TO_MPS(KMPH) (KMPH / 3.6)
#define KMPH_TO_MPH(KMPH) (KMPH / 1.609344)

#define input_Anemometer PA3
#define maxInput_windSpeed 2.38   // seconds
#define maxKnown_windSpeed 407.16 // km/h
#define dataPointsPerMeasurement 10   // Number of data points per measurement to take the final mean value

#define timeBetweenReadingPeriods 0.5   // minutes
unsigned long idleTimeInMilliseconds;

unsigned long EdgeTiming_Overflow = 0;
uint8_t CalmAirTiming_Overflow = 0,
        CalmAirTiming_MaxValue;

TIM_TypeDef *Instance_TIM1 = TIM1;   
HardwareTimer *EdgePeriodTimer = new HardwareTimer(Instance_TIM1);

TIM_TypeDef *Instance_TIM2 = TIM2;   
HardwareTimer *CalmAirTimer = new HardwareTimer(Instance_TIM2);

bool idlePeriod = true,
     readFlag = false;
     
bool isTakingFirstEdge = false,
     isSecondEdgeDetected  = false;

bool CalmAir = false;

float windSpeed[dataPointsPerMeasurement];
uint8_t arr_index = 0;

float meanWindSpeed = 0;
uint8_t fault_count = 0;

unsigned long timestamp;

void setup() {
  Serial.begin(9600);
  pinMode(input_Anemometer, INPUT);
  pinMode(PC13, OUTPUT);

  idleTimeInMilliseconds = timeBetweenReadingPeriods * 60E3;
  CalmAirTiming_MaxValue = (uint8_t)(maxInput_windSpeed / (10E-3));

  EdgePeriodTimer->setMode(1, TIMER_DISABLED);    // Use channel 1 of TIM1 in output compare mode, no output
  // EdgePeriodTimer->setPrescaleFactor(72);      // Prescaler = 72 => TimClk = SysClk / 72 = 72MHz / 72 = 1MHz => Timer counter increments every 1us
                                                  // setPrescaleFactor() is not necessary since setOverflow() is called with MICROSEC_FORMAT, thus automatically updated timer clock
  EdgePeriodTimer->setOverflow(10000, MICROSEC_FORMAT);   // Timer overflows every 10ms
  EdgePeriodTimer->attachInterrupt(1, std::bind(TIM_Ovf_callback, EdgePeriodTimer));  // ISR run whenever timer overflows for channel 1

  CalmAirTimer->setMode(2, TIMER_DISABLED);    // Use channel 1 of TIM2 in output compare mode, no output
  CalmAirTimer->setOverflow(10000, MICROSEC_FORMAT);      // Timer overflows every 10ms
  CalmAirTimer->attachInterrupt(2, std::bind(TIM_Ovf_callback, CalmAirTimer));  // ISR run whenever timer overflows for channel 1

  readFlag = false;
  timestamp = millis();
}

void loop() {  
  static bool LED_STATE = false;
  if (!readFlag && idlePeriod) {
    Serial.print(meanWindSpeed);
    Serial.println("km/h\n");

    delay(1000);
    
    readFlag = true;
    Serial.println("\nNew cycle.");
  }

  Anemometer_Reading_Routine();

  digitalWrite(PC13, LED_STATE);
  if (millis() - timestamp >= 1000) {
    timestamp = millis();
    LED_STATE = !LED_STATE;
  }
}

void Anemometer_Reading_Routine(void) {
  if (readFlag && idlePeriod) {   // A new reading routine begins
    idlePeriod = false;
    arr_index = 0;
    fault_count = 0;

    Initialise_New_Timing_Period();
  }
}

void Initialise_New_Timing_Period(void) {
  isTakingFirstEdge = true;
  isSecondEdgeDetected = false;
  CalmAir = false;

  EdgePeriodTimer->pause();  // Make sure the timer is not running, then reset counter register
  EdgePeriodTimer->setCount(0);

  CalmAirTimer->pause();    // Make sure the timer is not running, then reset counter register
  CalmAirTimer->setCount(0);

  attachInterrupt(digitalPinToInterrupt(input_Anemometer), anemometerInput_Detected, RISING);
  CalmAirTimer->resume();
}

void WindSpeed_Array_Update_Routine(void) {
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

void WindSpeed_MeanValue_Update_Routine(void) {
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
  else {
    NOP;
  }

  // End of the reading routine
  readFlag = false;
  idlePeriod = true;
}

void TIM_Ovf_callback(HardwareTimer* OverflownTimer) {
  if (EdgePeriodTimer == OverflownTimer) {
    EdgeTiming_Overflow += 1;
  }
  else if (CalmAirTimer == OverflownTimer) {
    CalmAirTiming_Overflow += 1;

    if (CalmAirTiming_MaxValue <= CalmAirTiming_Overflow) {
      EdgePeriodTimer->pause();         // Pause (running) timers
      CalmAirTimer->pause();

      detachInterrupt(digitalPinToInterrupt(input_Anemometer));   // Stop taking inputs

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

void anemometerInput_Detected(void) {
  CalmAirTimer->pause();    // Pause the running CalmAir timer
  
  if (!isTakingFirstEdge) {
    EdgePeriodTimer->pause();   // Pause running EdgePeriod timer
    
    detachInterrupt(digitalPinToInterrupt(input_Anemometer));   // Stop taking inputs
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
