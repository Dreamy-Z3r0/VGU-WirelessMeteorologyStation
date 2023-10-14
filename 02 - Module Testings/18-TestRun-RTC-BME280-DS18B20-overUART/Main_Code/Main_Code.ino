#include "Inc/Libraries.h"
#include "Inc/Macros_and_Defines.h"
#include "Inc/Class_Instances.h"

bool firstRun = true;
unsigned long dataIndex = 0;

Alarm_Type RTC_Alarm_Flag = NO_ALARM;
uint8_t customPeriod_count = 0;
String DateData = "11-09-2023", 
       TimeData = "08:58:00";

void setup() {
  Wire.begin();

  #if defined(DEBUGGING_OVER_SERIAL)
  Serial.begin(BAUD_RATE);
  #elif defined(SERIAL_MONITOR_OUTPUT)
  Serial.begin(BAUD_RATE);
  #elif defined(RASPBERRY_PI_LOGGING)
  Serial.begin(BAUD_RATE);
  #endif

  delay(2000);

  // Initialisation
  RTC_DS3231.init();
  Project_Sensor.init(); 
  
  delay(100);   // Delay a short period for the system to fully initialise

  // Date - time setup
  RTC_DS3231.DateTime_InputString_Processing("Date:" + DateData);
  RTC_DS3231.DateTime_InputString_Processing("Time:" + TimeData);

  String SystemConfigChange = "";
  SystemConfigChange += ("    Date updated to " + DateData + "\n");
  SystemConfigChange += ("    Time updated to " + TimeData + "\n");
}

void loop() {
  DateTime_InputHandler();
  
  if (firstRun) {
    Serial.printf("\n(%d) ", dataIndex);
    DateTimeDisplay();    // Print date time
    Serial.printf("\n");

    Project_Sensor.Sensor_Control_Readings_Handler();   // Update sensor readings
    Project_Sensor.Sensor_Data_Printout();    // Print sensor readings
 
    dataIndex += 1;
    firstRun = false;
  }
  
  RTC_Alarm_Flag = RTC_DS3231.alarm_status();
  
  switch (RTC_Alarm_Flag) {
    case DAILY_ALARM: {
      Serial.printf("(%d) ", dataIndex);
      DateTimeDisplay();
      Serial.printf(" - Daily alarm\n");
      break;
    }
    case MINUTE_ALARM: {
      if (CUSTOM_MINUTE_ALARM > 1) {
        customPeriod_count += 1;
        if (CUSTOM_MINUTE_ALARM == customPeriod_count) {
          customPeriod_count = 0;
          Serial.printf("(%d) ", dataIndex);
          DateTimeDisplay();
          Serial.printf("\n");
        } else {
          RTC_Alarm_Flag = NO_ALARM;
        }
      } else {
        Serial.printf("(%d) ", dataIndex);
        DateTimeDisplay();
        Serial.printf("\n");
      }
      break;
    }
    case DUAL_ALARM: {
      if (CUSTOM_MINUTE_ALARM > 1) {
        customPeriod_count += 1;
        if (CUSTOM_MINUTE_ALARM == customPeriod_count) {
          customPeriod_count = 0;
          Serial.printf("(%d) ", dataIndex);
          DateTimeDisplay();
          Serial.printf(" - Daily alarm+\n");
        } else {
          RTC_Alarm_Flag = DAILY_ALARM;
          Serial.printf("(%d) ", dataIndex);
          DateTimeDisplay();
          Serial.printf(" - Daily alarm\n");
        }
      } else {
        Serial.printf("(%d) ", dataIndex);
        DateTimeDisplay();
        Serial.printf(" - Daily alarm+\n");
      }
      break;
    }
    default: {
      break;
    }
  }

  if (NO_ALARM != RTC_Alarm_Flag) {
    Project_Sensor.Sensor_Control_Readings_Handler();   // Update sensor readings
    Project_Sensor.Sensor_Data_Printout();    // Print sensor readings
    
    dataIndex += 1;
  }
}

void DateTime_InputHandler() {
  #ifdef SERIAL_MONITOR_OUTPUT
  if (Serial.available()) {   // Check for user input
    String input = Serial.readStringUntil('\n');

    if (input.substring(0,4).equals("Date") | input.substring(0,4).equals("Time")) {
      RTC_DS3231.DateTime_InputString_Processing(input);
    } else {
      Serial.println("Unrecognised input data.");
    }
  }
  #endif
}

void DateTimeDisplay(void) {
  RTC_DS3231.readRTC();

  int RTC_data_holder;
  DateData = "";
  TimeData = "";

  RTC_data_holder = RTC_DS3231.readDay();
  DateData += (RTC_data_holder < 10 ? "0" : "");
  DateData += (String(RTC_data_holder) + "/");

  RTC_data_holder = RTC_DS3231.readMonth();
  DateData += (RTC_data_holder < 10 ? "0" : "");
  DateData += (String(RTC_data_holder) + "/");

  RTC_data_holder = RTC_DS3231.readYear();
  DateData += String(RTC_data_holder);

  RTC_data_holder = RTC_DS3231.readHour();
  TimeData += (RTC_data_holder < 10 ? "0" : "");
  TimeData += (String(RTC_data_holder) + ":");

  RTC_data_holder = RTC_DS3231.readMinute();
  TimeData += (RTC_data_holder < 10 ? "0" : "");
  TimeData += (String(RTC_data_holder) + ":");

  RTC_data_holder = RTC_DS3231.readSecond();
  TimeData += (RTC_data_holder < 10 ? "0" : "");
  TimeData += String(RTC_data_holder);

  Serial.printf("%s %s", DateData.c_str(), TimeData.c_str());
}
