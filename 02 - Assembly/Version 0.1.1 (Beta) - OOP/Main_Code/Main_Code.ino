#include "Libraries.h"
#include "Macros_and_Variables.h"

void setup() {
  #ifdef DEBUGGING_OVER_SERIAL
  Serial.begin(BAUD_RATE);
  #endif

  Wire.begin();

  HardwareTimer *fetch_RTC = new HardwareTimer(TIM4);
  fetch_RTC->setOverflow(1, HERTZ_FORMAT);  // callback runs every 1 second
  fetch_RTC->attachInterrupt(std::bind(request_from_RTC, &RTC_DS3231));
  fetch_RTC->resume();
}

void loop() {
  DateTime_InputHandler(); 
  DateTime_Display();
}

void DateTime_InputHandler() {
  #ifdef DEBUGGING_OVER_SERIAL
  if (Serial.available()) {   // Check for user input
    String input = Serial.readStringUntil('\n');
    bool inputVerification;
    
    String input_processed = input.substring(0, input.indexOf(':'));  // Identifier of input data
    if (input_processed.equals("Date")) {    // Input is date data
      if (15 != input.length()) {   // Expected input length for DD-MM-YYYY format
        Serial.println("Invalid format. Date should be input in DD-MM-YYYY format.");
      } else {
        input_processed = input.substring(5);   // Extract the data string
        RTC_DS3231.input_DateTime(input_processed, day|month|year, &inputVerification);  // Process the extracted data and verify
        if (inputVerification) RTC_DS3231.update_RTC(day|month|year);  // Update RTC if the extracted data is valid
      }
    } else if (input_processed.equals("Time")) {   // Input is time data
      if ((13 == input.length()) || (10 == input.length())) {   // Expected input lengths for HH:MM:SS and HH:MM formats
        input_processed = input.substring(5);   // Extract the data string

        if (13 == input.length()) {   // HH:MM:SS format
          RTC_DS3231.input_DateTime(input_processed, hour|minute|second, &inputVerification);  // Process the extracted data and verify
          if (inputVerification) RTC_DS3231.update_RTC(hour|minute|second);  // Update RTC if the extracted data is valid
        } else {    // HH:MM format
          RTC_DS3231.input_DateTime(input_processed, hour|minute, &inputVerification);  // Process the extracted data and verify
          if (inputVerification) RTC_DS3231.update_RTC(hour|minute);  // Update RTC if the extracted data is valid
        }
      } else {
        Serial.println("Invalid format. Time should be input in HH:MM or HH:MM:SS formats.");
      }
    } else {  // Invalid data input
      Serial.print("Unrecogised input identifier: ");
      Serial.println(input.substring(0,4));
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
    Serial.print(RTC_data < 10 ? "0" : "\0");
    Serial.print(RTC_data);

    // Print month
    RTC_data = RTC_DS3231.readMonth();
    Serial.print(RTC_data < 10 ? "/0" : "/");
    Serial.print(RTC_data);

    // Print year
    RTC_data = RTC_DS3231.readYear();
    Serial.print("/");
    Serial.print(RTC_data);

    // Print hour
    RTC_data = RTC_DS3231.readHour();
    Serial.print(RTC_data < 10 ? " 0" : " ");
    Serial.print(RTC_data);

    // Print minute
    RTC_data = RTC_DS3231.readMinute();
    Serial.print(RTC_data < 10 ? ":0" : ":");
    Serial.print(RTC_data);

    // Print second
    RTC_data = RTC_DS3231.readSecond();
    Serial.print(RTC_data < 10 ? ":0" : ":");
    Serial.print(RTC_data);

    // Check and print the validity of RTC data
    DS3231 RTC_quality;
    Serial.println(RTC_quality.oscillatorCheck() ? " (ok)" : "!");
    
    #endif
  }
}
