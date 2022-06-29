#include "Libraries.h"
#include "Macros_and_Variables.h"

void setup() {
  #ifdef DEBUGGING_OVER_SERIAL
  Serial.begin(BAUD_RATE);
  #endif

  SPI.setMOSI(default_MOSI_Pin);
  SPI.setMISO(default_MISO_Pin);
  SPI.setSCLK(default_SCLK_Pin);
  SPI.setSSEL(default_SSEL_Pin);

  Wire.begin();

//  pinMode(SD_CS_Pin, OUTPUT);
//  if (!SD.begin(SD_CS_Pin)) {
//    return;     // do nothing more
//  }

  if (!bme.begin()) {
    Serial.println("Problem initializing BME280.");
    return;
  }

  HardwareTimer *fetch_RTC = new HardwareTimer(TIM4);
  fetch_RTC->setOverflow(1, HERTZ_FORMAT);  // callback runs every 1 second
  fetch_RTC->attachInterrupt(std::bind(request_from_RTC, &readRTC_control_flag));
  fetch_RTC->resume();

  readRTC(&myRTC, RTC_data);

  BME280_userSettings.Power_Mode = Adafruit_BME280::MODE_NORMAL;
  BME280_userSettings.Temperature_Oversampling = Adafruit_BME280::SAMPLING_X2;
  BME280_userSettings.Pressure_Oversampling = Adafruit_BME280::SAMPLING_X16;
  BME280_userSettings.Humidity_Oversampling = Adafruit_BME280::SAMPLING_X1;
  BME280_userSettings.Filter_Coefficient = Adafruit_BME280::FILTER_X16;
  BME280_userSettings.readInterval = 1000;
  
  update_BME280_settings(&bme, &BME280_userSettings);
  startup_timestamp = millis();
}

void loop() {
  DateTime_InputHandler();
  
  if (readRTC_control_flag) {
    readRTC_control_flag = false;
    BME280_readFlag = true;

    DateTime_Display();
  }
  
  #ifdef BME280_starting
  if (120000 <= (millis() - startup_timestamp)) {
    #undef BME280_starting
  } else if (BME280_readFlag) {
    BME280_readFlag = false;
    Serial.println("     BME280 is in startup period...\n");
  }
  #endif
  
  BME280_Update();
}

void DateTime_InputHandler() {
  if (Serial.available()) {   // Check for user input
    String input = Serial.readStringUntil('\n');
    bool inputVerification;
    
    String input_processed = input.substring(0,5);  // Identifier of input data
    if (input_processed.equals("Date:")) {    // Input is date data
      if (15 != input.length()) {   // Expected input length for DD-MM-YYYY format
        Serial.println("Invalid format. Date should be input in DD-MM-YYYY format.");
      } else {
        input_processed = input.substring(5);   // Extract the data string
        input_DateTime(input_processed, RTC_data, day|month|year, &inputVerification);  // Process the extracted data and verify
        if (inputVerification) update_RTC(RTC_data, day|month|year);  // Update RTC if the extracted data is valid
      }
    } else if (input_processed.equals("Time:")) {   // Input is time data
      if ((13 == input.length()) || (10 == input.length())) {   // Expected input lengths for HH:MM:SS and HH:MM formats
        input_processed = input.substring(5);   // Extract the data string

        if (13 == input.length()) {   // HH:MM:SS format
          input_DateTime(input_processed, RTC_data, hour|minute|second, &inputVerification);  // Process the extracted data and verify
          if (inputVerification) update_RTC(RTC_data, hour|minute|second);  // Update RTC if the extracted data is valid
        } else {    // HH:MM format
          input_DateTime(input_processed, RTC_data, hour|minute, &inputVerification);  // Process the extracted data and verify
          if (inputVerification) update_RTC(RTC_data, hour|minute);  // Update RTC if the extracted data is valid
        }
      } else {
        Serial.println("Invalid format. Time should be input in HH:MM or HH:MM:SS formats.");
      }
    } else {  // Invalid data input
      Serial.print("Unrecogised input identifier: ");
      Serial.println(input.substring(0,4));
    }
  }
}

void DateTime_Display() {
    // Get calendar and time data
    readRTC(&myRTC, RTC_data);

    // Print day
    Serial.print(RTC_data[0] < 10 ? "0" : "\0");
    Serial.print(RTC_data[0]);

    // Print month
    Serial.print(RTC_data[1] < 10 ? "/0" : "/");
    Serial.print(RTC_data[1]);

    // Print year
    Serial.print("/");
    Serial.print(RTC_data[2]);

    // Print hour
    Serial.print(RTC_data[3] < 10 ? " 0" : " ");
    Serial.print(RTC_data[3]);

    // Print minute
    Serial.print(RTC_data[4] < 10 ? ":0" : ":");
    Serial.print(RTC_data[4]);

    // Print second
    Serial.print(RTC_data[5] < 10 ? ":0" : ":");
    Serial.print(RTC_data[5]);

    DS3231 RTC_quality;
    Serial.println(RTC_quality.oscillatorCheck() ? " (ok)" : "!");
}

void BME280_Update() {
  if (BME280_readFlag) {
    read_BME280(&bme, &BME280_userSettings, &BME280_dataStorage, &BME280_readFlag);

    if (!BME280_readFlag) {
      Serial.print("     Ambient temperature = ");
      Serial.print(BME280_dataStorage.temperature);
      Serial.println(" ºC");

      Serial.print("     Relative humidity = ");
      Serial.print(BME280_dataStorage.humidity);
      Serial.println(" %RH");

      Serial.print("     Atmospheric pressure = ");
      Serial.print(BME280_dataStorage.pressure);
      Serial.println(" hPa");

      Serial.println();
    }
  }
}
