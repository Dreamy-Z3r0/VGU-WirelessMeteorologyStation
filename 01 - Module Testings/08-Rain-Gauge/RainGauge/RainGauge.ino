/******************************************************************
 ******************************************************************
 ***   Rain gauge test for STM32F103CBT6 on Arduino framework   ***
 *** ---------------------------------------------------------- ***
 ***                                                            ***
 *** The rain gauge in use is a self-emptying tipping bucket    ***
 *** that gives of a pulse for every 0.2794 mm of rainfall. The ***
 *** data is recorded daily at 9am. The time is monitored by a  ***
 *** DS3231 Real-time Clock, communicating with the microcon-   ***
 *** troller over I2C interface and a hardware interrupt pin.   ***
 ***                                                            ***
 *** On the STM32F103CBT6, the default I2C is mapped to         ***
 *** (SDA1, SCL1) = (PB7, PB6). Any free digital pin could be   ***
 *** used as interrupt pin for daily alarm from DS3231.         ***
 ***                                                            ***
 *** DS3231 has 2 available alarm, only one of which is used.   *** 
 *** The INT/SQW pin on the module is the alarm pin, and is     *** 
 *** active-low, which means when there is an alarm (at 9am in  ***
 *** this project), there shall be a falling edge for the       *** 
 *** microcontroller to detect and to run a corresponding       ***
 *** interrupt service routine.                                 ***
 ***                                                            ***
 ******************************************************************
 ******************************************************************/

#include <Wire.h>
#include <DS3231.h>

#define AlarmInput PB8

#define input_RainGauge PB3
#define RainfallPerTip  0.2794  // mm

unsigned long Daily_Counter = 0;
double Rainfall_Data;
bool RainGauge_DailyAlarm;

void setup() {
  Serial.begin(9600);
  while (!Serial);
  Wire.begin();   // Starting I2C
  
  pinMode(AlarmInput, INPUT_PULLUP);    // Initialize alarm pin
  
  DS3231 clock;
  clock.setA1Time(1, 9, 0, 0, 0x8, false, false, false);    // Set up daily alarm at 9 am sharp
  clock.turnOnAlarm(1);     // Enable alarm 1

  if (!digitalRead(AlarmInput)) {   // Make sure there is no alarm present initially
    clock.checkIfAlarm(1);
  }

  attachInterrupt(digitalPinToInterrupt(AlarmInput), Alarm_Callback, FALLING);    // Enable interrupt from alarm
  
  pinMode(input_RainGauge, INPUT_PULLUP);  // Initialize rain gauge input
  attachInterrupt(digitalPinToInterrupt(input_RainGauge), RainGauge_Tipping_Callback, FALLING);   // Routine on a bucket tip
}

void loop() {   
  if (RainGauge_DailyAlarm) {
    unsigned long Lastest_Record = Daily_Counter;   // Store Daily_Counter to a temporary variable to reset it
    Daily_Counter = 0;                              // for the next daily count in case it's still counting.

    Rainfall_Data = RainfallPerTip * Lastest_Record;    // Calculate the rainfall amount of the previous day

    Serial.print("Precipitation data (rainfall): ");    // Print out precipitation data of the previous day
    Serial.print(Rainfall_Data, 4);
    Serial.println(" mm");

    RainGauge_DailyAlarm = false;   // Clear daily alarm
  }
}

void RainGauge_Tipping_Callback(void) {
  Daily_Counter += 1;
}

void Alarm_Callback(void) {
  DS3231 alarm;
  bool alarm_1 = alarm.checkIfAlarm(1);

  if (alarm_1) {
    RainGauge_DailyAlarm = true;
  }
}
