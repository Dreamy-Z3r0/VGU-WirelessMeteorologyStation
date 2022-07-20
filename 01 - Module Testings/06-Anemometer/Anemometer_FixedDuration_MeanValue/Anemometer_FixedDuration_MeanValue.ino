/*********************************************************************
 *********************************************************************
 ***    Anemometer test for STM32F103CBT6 on Arduino framework     ***
 *** ------------------------------------------------------------- ***
 ***                                                               ***
 *** An anemometer is used for measuring wind speed. The sensor    ***
 *** is simply a reed switch that closes every time the cups make  ***
 *** a full rotation due to the wind. The methods of reading data  ***
 *** from an anemometer are fairly simple, either to count the     ***
 *** number of rotations during a fixed period (for example, 5s),  ***
 *** or to time 1 single rotation.                                 ***
 ***                                                               ***
 *** Anemometer_FixedDuration tests the first method. The fixed    ***
 *** duration is pre-defined at the very beginning of the program. ***
 *** Sampling routine is called after a fixed period from the last ***
 *** one, and the counter is incremented whenever the switch       ***
 *** (anemometer's reed switch) toggles, which gives 2 edges per   ***
 *** revolution instead of 1. The wind speed is then calculated as ***
 *** followed:                                                     ***                                                            
 ***    windSpeed = (inputCounter / (2 * duration)) * (2/3) (m/s)  ***                                                            
 ***              =  inputCounter / (3 * duration)          (m/s)  ***    
 ***                                                               ***                                        
 *** Note 1: According to the datasheet, 1 rotation per second (or ***                                                            
 ***         2 edges per second in this program) is equivalent to  ***
 ***         a wind of 2.4 km/h (2/3 m/s).                         ***                                                  
 ***                                                               ***    
 *** Note 2: Any external interrupt pin could be used as input pin ***                                                            
 ***         for the anemometer.                                   ***                                                               
 ***                                                               ***        
 *** Note 3: This code utilizes the technique of taking mean value ***                                                          
 ***         by reading a fixed number of n-second wind speed data ***
 ***         continuously over 1 sampling period.                  ***
 ***                                                               ***
 *********************************************************************
 *********************************************************************/

#define MPS_TO_KMPH(MPS) (MPS * 3.6)                      // conversion from m/s to km/h
#define MPS_TO_MPH(MPS)  (MPS_TO_KMPH(MPS) / 1.609344)    // conversion from m/s to mph

#define input_Anemometer PA3
#define timeBetweenReadingPeriods 1  // minutes
unsigned long idleTimeInMilliseconds;

#define rawSampleQuantity 20    // number of sampling slots per sensor reading period
#define sampleDuration 5        // seconds per sample
uint8_t secondCounter = 0;

unsigned int rawData[rawSampleQuantity];
uint8_t rawData_index = 0;
bool rawData_ready = false;

double windSpeed;
bool sensorReadingRequested = false;
bool idlePeriod = false;

TIM_TypeDef *Instance = TIM1;
HardwareTimer *InputRoutineTim = new HardwareTimer(Instance);

unsigned long timestamp;

void setup() {
  Serial.begin(9600);
  idleTimeInMilliseconds = timeBetweenReadingPeriods * 60000;

  pinMode(input_Anemometer, INPUT);

  InputRoutineTim->setOverflow(1, HERTZ_FORMAT);
  InputRoutineTim->attachInterrupt(samplingTimeSlot);

  sensorReadingRequested = true;
}

void loop() {
  if (idlePeriod) {
    if (timestamp > millis()) {   // request a new wind speed update in case timer for millis() overflows
      idlePeriod = false;
      sensorReadingRequested = true;
    }
    else if ((millis() - timestamp) >= idleTimeInMilliseconds) {  // request a new wind speed update after [timeBetweenReadingPeriods]-minute idle period
      idlePeriod = false;
      sensorReadingRequested = true;
    }
  }
  
  if (sensorReadingRequested) {
    sensorReadingRequested = false;  // clear request
    
    rawData[rawData_index] = 0;      // reset raw data at array index 0
    InputRoutineTim->setCount(0);    // reset timer counter
    secondCounter = 0;               // make sure second counter starts at 0
    
    InputRoutineTim->resume();       // start a new sensor reading period
    attachInterrupt(digitalPinToInterrupt(input_Anemometer), anemometerEdgeDetected, CHANGE);
  }

  if (rawData_ready) {
    idlePeriod = true;
    timestamp = millis();

    rawData_ready = false;
    windSpeed_update();

    Serial.print("Wind speed: ");
    
    Serial.print(windSpeed, 2);     // wind speed in m/s
    Serial.print(" m/s - ");

    Serial.print(MPS_TO_KMPH(windSpeed), 2);     // wind speed in km/h
    Serial.print(" km/h - ");

    Serial.print(MPS_TO_MPH(windSpeed), 2);     // wind speed in mph
    Serial.println(" mph");
  }
}

void anemometerEdgeDetected(void) {
  rawData[rawData_index] += 1;
}

void samplingTimeSlot() {     // time the [sampleDuration] second slot of 1 raw data
  secondCounter += 1;
  
  if (sampleDuration == secondCounter) {    // a sampling time slot is up
    InputRoutineTim->pause();       // pause timer
    detachInterrupt(digitalPinToInterrupt(input_Anemometer));   // stop taking inputs
    
    InputRoutineTim->setCount(0);   // reset timer counter
    
    secondCounter = 0;      // reset second counter
    rawData_index += 1;     // increment raw data array index
    
    if (rawSampleQuantity == rawData_index) {   // raw data array is full 
      rawData_index = 0;      // reset array index
      rawData_ready = true;   // indicates raw data array is full
    }
    else {
      rawData[rawData_index] = 0;   // reset raw data holder slot
      InputRoutineTim->resume();    // resume timer counter
      attachInterrupt(digitalPinToInterrupt(input_Anemometer), anemometerEdgeDetected, CHANGE);   // resume sampling raw data
    }
  }
}

void windSpeed_update(void) {   // update the wind speed value after the total sampling period of [rawSampleQuantity] * [sampleDuration] seconds is over
  windSpeed = 0;
  for (uint8_t i = 0; i < rawSampleQuantity; i += 1) {    // take the sum of [sampleDuration]-second wind speed values from raw data
    windSpeed += (rawData[i] / (3.0 * sampleDuration));
  }
  windSpeed /= rawSampleQuantity;   // take the mean value of wind speed over the [rawSampleQuantity] samples
}
