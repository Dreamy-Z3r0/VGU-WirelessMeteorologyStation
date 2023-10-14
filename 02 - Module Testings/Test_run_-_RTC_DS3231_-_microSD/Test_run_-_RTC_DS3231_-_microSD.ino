#include "RTC.h"
//#include "microSD.h"

bool firstRun = true;
unsigned long dataIndex = 0;

#define TEMPERATURE_RANDOM_LOW -40
#define TEMPERATURE_RANDOM_HIGH -40

#define PRESSURE_RANDOM_LOW 950
#define PRESSURE_RANDOM_HIGH 1050

#define HUMIDITY_RANDOM_HIGH 100

long randomTemperatureGround, randomTemperatureAir, randomPressure, randomHumidity;

//#define microSD_SS PA4
//Card_Access microSD(microSD_SS);

Alarm_Type RTC_Alarm_Flag = NO_ALARM;
uint8_t customPeriod_count = 0;
String DateData = "11-09-2023", 
       TimeData = "08:58:00";

void setup() {
  Wire.begin();
  Serial.begin(9600);
  delay(2000);
  Serial.println("Serial begun.\n");

  // Initialisation
//  microSD.init();
  RTC_DS3231.init();

  // Log initialisation
//  microSD.add_entry(ChangeLog, "Boot.");

  // Date - time setup
  RTC_DS3231.DateTime_InputString_Processing("Date:" + DateData);
  RTC_DS3231.DateTime_InputString_Processing("Time:" + TimeData);

  String SystemConfigChange = "";
  SystemConfigChange = String(SystemConfigChange + "    Date updated to " + DateData + "\n");
  SystemConfigChange = String(SystemConfigChange + "    Time updated to " + TimeData + "\n");

  // Update log file: SystemConfig <- updated date and time
//  microSD.add_entry(SystemConfig, String("Date and time changed: \n" + SystemConfigChange));
}

void loop() {
  String DailyLogChange = "";
  
  if (firstRun) {
    firstRun = false;
    Serial.printf("\n(%d) ", dataIndex);
    DateTimeDisplay();
    Serial.printf("\n");
    
//    LoggingPseudoroutine(&DailyLogChange);
    DailyLogChange = "";
    dataIndex += 1;
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
//    LoggingPseudoroutine(&DailyLogChange);
    dataIndex += 1;
  }
}

void DateTimeDisplay(void) {
  RTC_DS3231.readRTC();

  int RTC_data_holder;
  DateData = "";
  TimeData = "";

  RTC_data_holder = RTC_DS3231.readDay();
  DateData = String(DateData + (RTC_data_holder < 10 ? "0" : ""));
  DateData = String(DateData + String(RTC_data_holder) + "/");

  RTC_data_holder = RTC_DS3231.readMonth();
  DateData = String(DateData + (RTC_data_holder < 10 ? "0" : ""));
  DateData = String(DateData + String(RTC_data_holder) + "/");

  RTC_data_holder = RTC_DS3231.readYear();
  DateData = String(DateData + String(RTC_data_holder));

  RTC_data_holder = RTC_DS3231.readHour();
  TimeData = String(TimeData + (RTC_data_holder < 10 ? "0" : ""));
  TimeData = String(TimeData + String(RTC_data_holder) + ":");

  RTC_data_holder = RTC_DS3231.readMinute();
  TimeData = String(TimeData + (RTC_data_holder < 10 ? "0" : ""));
  TimeData = String(TimeData + String(RTC_data_holder) + ":");

  RTC_data_holder = RTC_DS3231.readSecond();
  TimeData = String(TimeData + (RTC_data_holder < 10 ? "0" : ""));
  TimeData = String(TimeData + String(RTC_data_holder));

//  Serial.printf("%s %s", DateData, TimeData);
  Serial.print(DateData);
  Serial.print(" ");
  Serial.print(TimeData);
}

//void LoggingPseudoroutine(String* Pseudochanges) {
//  // Generate random values
//  GenerateRandomValues();
//
//  // Update daily log
//  *Pseudochanges = String(*Pseudochanges + DateData + "," + TimeData + ",");
//  *Pseudochanges = String(*Pseudochanges + String(dataIndex) + ",");
//  
//  if (DAILY_ALARM == RTC_Alarm_Flag) {
//    *Pseudochanges = String(*Pseudochanges + "Daily alarm,");
//  } else if (MINUTE_ALARM == RTC_Alarm_Flag) {
//    if (CUSTOM_MINUTE_ALARM > 1) {
//      *Pseudochanges = String(*Pseudochanges + "Custom period alarm,");
//    } else {
//      *Pseudochanges = String(*Pseudochanges + "Minute alarm,");
//    }
//  } else if (DUAL_ALARM == RTC_Alarm_Flag) {
//    *Pseudochanges = String(*Pseudochanges + "Daily alarm+,");
//  } else {
//    *Pseudochanges = String(*Pseudochanges + "NONE (first run),");
//  }
//  
//  *Pseudochanges = String(*Pseudochanges + String(randomTemperatureGround) + "," + String(randomTemperatureAir) + ",");
//  *Pseudochanges = String(*Pseudochanges + String(randomHumidity) + "," + String(randomPressure) + "\n");
//
//  microSD.add_entry(DailyLog, *Pseudochanges);
//}

void GenerateRandomValues() {
  randomTemperatureGround = random(TEMPERATURE_RANDOM_LOW, TEMPERATURE_RANDOM_HIGH + 1);
  randomTemperatureAir = random(TEMPERATURE_RANDOM_LOW, TEMPERATURE_RANDOM_HIGH + 1);
  randomPressure = random(PRESSURE_RANDOM_LOW, PRESSURE_RANDOM_HIGH + 1);
  randomHumidity = random(HUMIDITY_RANDOM_HIGH + 1);
}
