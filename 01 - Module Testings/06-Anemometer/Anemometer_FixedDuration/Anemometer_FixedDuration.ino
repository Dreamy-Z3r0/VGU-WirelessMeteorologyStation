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
 *** Sampling routine is called every minute, and the counter is   ***
 *** incremented whenever the switch (anemometer's reed switch)    ***
 *** toggles, which gives 2 edges per revolution instead of 1. The ***                                                            
 *** wind speed is then calculated as followed:                    ***                                                            
 ***    windSpeed = (inputCounter / (2 * duration)) * (2/3) (m/s)  ***                                                            
 ***              =  inputCounter / (3 * duration)          (m/s)  ***    
 ***                                                               ***                                        
 *** Note 1: According to the datasheet, 1 rotation per second (or ***                                                            
 ***         2 edges per second in this program) is equivalent to  ***
 ***         a wind of 2.4 km/h (2/3 m/s).                         ***                                                  
 ***                                                               ***    
 *** Note 2: Any external interrupt pin could be used as input pin ***                                                            
 ***         for the anemometer.                                   ***                                                               
 *********************************************************************
 *********************************************************************/


#define input_Anemometer PB2
unsigned int inputCounter = 0;

#define duration 10
uint8_t Tracker_duration = 0;

uint8_t minuteTracker = 0;
bool newMinute = true;

void setup() {
  Serial.begin(9600);

  TIM_TypeDef *Instance = TIM1;
  HardwareTimer *InputRoutineTim = new HardwareTimer(Instance);

  InputRoutineTim->setOverflow(1, HERTZ_FORMAT);
  InputRoutineTim->attachInterrupt(std::bind(secondCounter, &minuteTracker));
  InputRoutineTim->resume();
}

void loop() {
  if (newMinute) {
    newMinute = false;
    RunEveryMinute();
  }
}

void secondCounter(uint8_t* counter) {
  *counter += 1;
  if (60 == *counter) 
  {
    *counter = 0;
    newMinute = true;
  }
}

void RunEveryMinute() {
  inputCounter = 0;
  Tracker_duration = 0;

  TIM_TypeDef *Instance = TIM2;
  HardwareTimer *InputTim = new HardwareTimer(Instance);

  InputTim->setOverflow(1, HERTZ_FORMAT);
  InputTim->attachInterrupt(std::bind(secondCounter, &Tracker_duration));
  InputTim->setCount(0);
  
  attachInterrupt(digitalPinToInterrupt(input_Anemometer), readAnemometer, CHANGE);
  InputTim->resume();
  
  while (Tracker_duration < duration);
  detachInterrupt(digitalPinToInterrupt(input_Anemometer));
  InputTim->pause();
  delete InputTim;

  float windSpeed = inputCounter / (duration * 3.0);
  Serial.print("Wind speed: ");
  Serial.print(windSpeed, 3);
  Serial.println(" m/s");
}

void readAnemometer() {
  inputCounter += 1;
}
