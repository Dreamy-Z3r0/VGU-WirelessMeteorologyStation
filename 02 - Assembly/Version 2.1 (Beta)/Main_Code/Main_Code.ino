#include "Inc/Libraries.h"
#include "Inc/Macros_and_Defines.h"
#include "Inc/Class_Instances.h"

void setup() {
  #ifdef DEBUGGING_OVER_SERIAL
  Serial.begin(BAUD_RATE);
  #endif

  Wire.begin();
  
  LoRa_Device.init();
  Project_Sensor.init();

  HardwareTimer *fetch_RTC = new HardwareTimer(TIM4);
  fetch_RTC->setOverflow(1, HERTZ_FORMAT);  // callback runs every 1 second
  fetch_RTC->attachInterrupt(std::bind(request_from_RTC, &RTC_DS3231));
  fetch_RTC->resume();
}

void loop() {
  DateTime_InputHandler();
  Project_Sensor.Sensor_Control_Main_Routine();
  
  if (RTC_DS3231.is_RTC_requested()) {   
    DateTime_Display();
    Project_Sensor.Sensor_Data_Printout();
  }

  if (LoRa_Device.Received_Message.messageStatus) {
    LoRa_Device.Received_Message.messageStatus = false;
    
    // Print out received message and its properties on debugging console
    #ifdef DEBUGGING_OVER_SERIAL
    Serial.print("~~~\nReceived message: ");
    Serial.println(LoRa_Device.Received_Message.message);

    Serial.printf("  package RSSI = %d dBm\n", LoRa_Device.Received_Message.packetRssi);
    Serial.printf("  package SNR = %d dB\n", LoRa_Device.Received_Message.packetSnr);
    Serial.printf("  Frequency error = %d Hz\n~~~\n\n", LoRa_Device.Received_Message.packetFrequencyError);
    #endif
  }
}

void DateTime_InputHandler() {
  #ifdef DEBUGGING_OVER_SERIAL
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

void DateTime_Display() {
  if (RTC_DS3231.is_RTC_requested()) {
    RTC_DS3231.clearRTC_request();
    
    // Get calendar and time data
    RTC_DS3231.readRTC();
    
    #ifdef DEBUGGING_OVER_SERIAL
    // Print day
    int RTC_data = RTC_DS3231.readDay();
    Serial.printf("%s%d", RTC_data < 10 ? "0" : "\0", RTC_data);

    // Print month
    RTC_data = RTC_DS3231.readMonth();
    Serial.printf("/%s%d", RTC_data < 10 ? "0" : "\0", RTC_data);

    // Print year
    RTC_data = RTC_DS3231.readYear();
    Serial.printf("/%d", RTC_data);

    // Print hour
    RTC_data = RTC_DS3231.readHour();
    Serial.printf(" %s%d", RTC_data < 10 ? "0" : "\0", RTC_data);

    // Print minute
    RTC_data = RTC_DS3231.readMinute();
    Serial.printf(":%s%d", RTC_data < 10 ? "0" : "\0", RTC_data);

    // Print second
    RTC_data = RTC_DS3231.readSecond();
    Serial.printf(":%s%d", RTC_data < 10 ? "0" : "\0", RTC_data);

    // Check and print the validity of RTC data
    DS3231 RTC_quality;
    Serial.println(RTC_quality.oscillatorCheck() ? " (ok)" : "!");

    #endif
  }
}
