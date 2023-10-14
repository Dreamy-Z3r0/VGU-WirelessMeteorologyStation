#include "Inc/Libraries.h"
#include "Inc/Macros_and_Defines.h"
#include "Inc/Class_Instances.h"

#define customPeriod 5  // minutes
String LoRa_Message_Field_Separator = ",";
String Timezone = "+07";

bool firstRun = true;
Alarm_Type RTC_Alarm_Flag = NO_ALARM;
uint8_t customPeriod_count = 0;
String DateData = "", 
       TimeData = "";

void setup() {
  Wire.begin();
  
  #if defined(DEBUGGING_OVER_SERIAL)
  Serial.begin(BAUD_RATE);
  delay(2000);
  #elif defined(SERIAL_MONITOR_OUTPUT)
  Serial.begin(BAUD_RATE);
  delay(2000);
  #elif defined(RASPBERRY_PI_LOGGING)
  Serial.begin(BAUD_RATE);
  delay(2000);
  #endif
  
  // Initialisation
  RTC_DS3231.init();
  Project_Sensor.init(); 
  
  delay(100);   // Delay a short period for the system to fully initialise
}

void loop() {
  // Check for and update date-time data via LoRa
  bool Rx_Msg_Status = false;
  LoRa_Received_Signal_Update(&Rx_Msg_Status);

  if (Rx_Msg_Status) {
    DateTime_InputHandler(LoRa_Device.Received_Message.message);
  }

  // Regular routine handling sensor data
  RTC_Alarm_Update();
  RTC_Alarm_Handler();
  LoRa_Communication_Handler();
}

void RTC_Alarm_Update(void) {
  RTC_Alarm_Flag = RTC_DS3231.alarm_status();
}

void RTC_Alarm_Handler() {
  switch (RTC_Alarm_Flag) {
    case MINUTE_ALARM: {
      customPeriod_count += 1;
      if (customPeriod == customPeriod_count) {   // Custom period reached (5 minutes)
        customPeriod_count = 0;                   // Reset the count variable
        Project_Sensor.Sensor_Control_Readings_Handler();     // Update all sensor data fields (except for rainfall)
        RTC_DS3231.readRTC();     // Fetch the date time data as timestamp for data point
      }
      break;
    }
    case DUAL_ALARM: {
      customPeriod_count += 1;
      if (customPeriod == customPeriod_count) {           // Custom period reached (5 minutes)
        Project_Sensor.Sensor_Control_Readings_Handler(ALL_SENSORS);  // Update all sensor data fields
        RTC_DS3231.readRTC();     // Fetch the date time data as timestamp for data point
      } else {
        Project_Sensor.Sensor_Control_Readings_Handler(ONLY_RAINFALL);  // Update only rainfall data
      }
      break;
    }
    case DAILY_ALARM: {
      Project_Sensor.Sensor_Control_Readings_Handler(ONLY_RAINFALL);  // Daily alarm present; update rainfall data
      break;
    }
    default: {  // NO_ALARM
      break;
    }
  }
}

void LoRa_Communication_Handler() {
  String outgoingMessage = "";
  
  switch (RTC_Alarm_Flag) {
    case MINUTE_ALARM: {
      LoRa_outgoingMessage_Update(&outgoingMessage);      // Update LoRa outgoing buffer
      LoRa_sendMessage(outgoingMessage);                  // Send message over Lora
      break;
    }
    case DUAL_ALARM: {
      if (customPeriod == customPeriod_count) {           // Custom period reached (5 minutes)
        customPeriod_count = 0;                           // Reset the count variable
        LoRa_outgoingMessage_Update(&outgoingMessage);    // Update LoRa outgoing buffer
        LoRa_sendMessage(outgoingMessage);                // Send message over Lora
      }
      break;
    }
    default: {  // NO_ALARM or DAILY_ALARM
      delay(1);
      break;
    }
  }
}

void DateTime_StringHandler(void) {
  int RTC_data_holder;
  DateData = "";
  TimeData = "";

  RTC_data_holder = RTC_DS3231.readYear();
  DateData += (String(RTC_data_holder) + "-");

  RTC_data_holder = RTC_DS3231.readMonth();
  DateData += (RTC_data_holder < 10 ? "0" : "");
  DateData += (String(RTC_data_holder) + "-");

  RTC_data_holder = RTC_DS3231.readDay();
  DateData += (RTC_data_holder < 10 ? "0" : "");
  DateData += (String(RTC_data_holder) + "-");

  RTC_data_holder = RTC_DS3231.readHour();
  TimeData += (RTC_data_holder < 10 ? "0" : "");
  TimeData += (String(RTC_data_holder) + ":");

  RTC_data_holder = RTC_DS3231.readMinute();
  TimeData += (RTC_data_holder < 10 ? "0" : "");
  TimeData += (String(RTC_data_holder) + ":");

  RTC_data_holder = RTC_DS3231.readSecond();
  TimeData += (RTC_data_holder < 10 ? "0" : "");
  TimeData += String(RTC_data_holder);

  TimeData += Timezone;
}

void LoRa_outgoingMessage_Update(String *outgoingMessage) {
  *outgoingMessage = "";

  // Write date time data to LoRa outgoing buffer
  DateTime_StringHandler();
  *outgoingMessage += (DateData + " " + TimeData);
  *outgoingMessage += LoRa_Message_Field_Separator;

  // Field [1]: wind speed
  *outgoingMessage += String(Project_Sensor.Latest_Sensor_Readings.Wind_Data[0]);
  *outgoingMessage += LoRa_Message_Field_Separator;

  // Field [2]: wind direction
  *outgoingMessage += String(Project_Sensor.Latest_Sensor_Readings.Wind_Data[1]);
  *outgoingMessage += LoRa_Message_Field_Separator;

  // Field [3]: temperature at the ground level
  *outgoingMessage += String(Project_Sensor.Latest_Sensor_Readings.DS18B20_Temperature);
  *outgoingMessage += LoRa_Message_Field_Separator;

  // Field [4]: temperature at 1m above the ground level
  *outgoingMessage += String(Project_Sensor.Latest_Sensor_Readings.BME280_Data[0]);
  *outgoingMessage += LoRa_Message_Field_Separator;

  // Field [5]: barometric pressure
  *outgoingMessage += String(Project_Sensor.Latest_Sensor_Readings.BME280_Data[1]);
  *outgoingMessage += LoRa_Message_Field_Separator;

  // Field [6]: relative humidity
  *outgoingMessage += String(Project_Sensor.Latest_Sensor_Readings.BME280_Data[2]);
  *outgoingMessage += LoRa_Message_Field_Separator;

  // Field [7]: rainfall
  *outgoingMessage += String(Project_Sensor.Latest_Sensor_Readings.Rainfall_Amount);
}

void LoRa_Received_Signal_Update(bool *Rx_Msg_Status) {
  *Rx_Msg_Status = LoRa_Device.Received_Message.messageStatus;    // Check for the availability of received message via LoRa
  if (*Rx_Msg_Status) {
    LoRa_Device.Received_Message.messageStatus = false;     // Clear the status flag before reading available message
  }
}

void DateTime_InputHandler(String input) {
  if (input.substring(0,4).equals("Date") | input.substring(0,4).equals("Time")) {
    RTC_DS3231.DateTime_InputString_Processing(input);
  } else {
    #ifdef SERIAL_MONITOR_OUTPUT
    Serial.println("Unrecognised input data.");
    #endif
  }
}
