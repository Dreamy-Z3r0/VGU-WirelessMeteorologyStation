#include "Libraries.h"
#include "Macros_and_Defines.h"

void setup() {
  #ifdef DEBUGGING_OVER_SERIAL
  Serial.begin(BAUD_RATE);
  #endif

  Wire.begin();

//  pinMode(SD_CS_Pin, OUTPUT);
//  if (!SD.begin(SD_CS_Pin)) {
//    return;     // do nothing more
//  }

  delay(2);
  if (!bme.begin()) {
    #ifdef DEBUGGING_OVER_SERIAL
    Serial.println("Problem initializing BME280.");
    #endif
    return;
  }
  delay(10);
  BME280_counter = 0;

  BME280_userSettings.Power_Mode = Adafruit_BME280::MODE_NORMAL;
  BME280_userSettings.Temperature_Oversampling = Adafruit_BME280::SAMPLING_X2;
  BME280_userSettings.Pressure_Oversampling = Adafruit_BME280::SAMPLING_X16;
  BME280_userSettings.Humidity_Oversampling = Adafruit_BME280::SAMPLING_X1;
  BME280_userSettings.Filter_Coefficient = Adafruit_BME280::FILTER_X16;
  BME280_userSettings.readInterval = 40;
  
  update_BME280_settings(&bme, &BME280_userSettings);

  update_DS18B20_bus(&DS18B20_userSettings, OneWireBus);
  update_DS18B20_settings(&DS18B20_userSettings, R_12BIT);

  HardwareTimer *fetch_RTC = new HardwareTimer(TIM4);
  fetch_RTC->setOverflow(1, HERTZ_FORMAT);  // callback runs every 1 second
  fetch_RTC->attachInterrupt(std::bind(request_from_RTC, &readRTC_control_flag));
  fetch_RTC->resume();
}

void loop() {
  DateTime_InputHandler();
  
  if (readRTC_control_flag) {
    DS18B20_readFlag = true;

    if (BME280_userSettings.readInterval < 1000) {
      BME280_readFlag = true;
    } else {
      BME280_counter += 1;
      if (BME280_userSettings.readInterval <= (BME280_counter * 1000)) {
        BME280_readFlag = true;
        BME280_counter = 0;
      }
    }
    
    DateTime_Display();
  }
  
  DS18B20_Update();
  BME280_Update();

  #ifdef DEBUGGING_OVER_SERIAL 
  if (newLine) {
    Serial.println("");
    newLine = false;
  }
  #endif
}

void DateTime_InputHandler() {
  #ifdef DEBUGGING_OVER_SERIAL
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
  #endif
}

void DateTime_Display() {
  if (readRTC_control_flag) {
    readRTC_control_flag = false;
    
    // Get calendar and time data
    readRTC(&myRTC, RTC_data);
    
    #ifdef DEBUGGING_OVER_SERIAL
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

    // Check and print the validity of RTC data
    DS3231 RTC_quality;
    Serial.println(RTC_quality.oscillatorCheck() ? " (ok)" : "!");

    // Add an empty line for visual purpose
    newLine = true;
    #endif
  }
}

void DS18B20_Update() {
  if (DS18B20_readFlag) {
    DS18B20_readFlag = false;   // Clear the request

    uint8_t present = 0;    // Guards the one-wire bus; if present == 0, no further action shall be taken within this scope.
    get_DS18B20_Temperature(&DS18B20_userSettings, &present, &DS18B20_Temperature);  // Gets sensor readings

    if (0 == present) {
      #ifdef DEBUGGING_OVER_SERIAL
      Serial.println("     (No device present on the One-Wire bus)");
      #endif
    } else {
      #ifdef DEBUGGING_OVER_SERIAL

      // Recycle the "present" variable for printing valid number of floating point values
      present = (uint8_t)(((DS18B20_userSettings.thermometerResolution >> 4) + 1) / 2); 

      // Print temperature
      Serial.print("     Ambient temperature = ");
      Serial.print(DS18B20_Temperature, present);

      switch (DS18B20_userSettings.thermometerResolution) {
        case R_9BIT:
          Serial.println(" ºC (9-bit DS18B20)");
          break;
        case R_10BIT:
          Serial.println(" ºC (10-bit DS18B20)");
          break;
        case R_11BIT:
          Serial.println(" ºC (11-bit DS18B20)");
          break;
        case R_12BIT:
          Serial.println(" ºC (12-bit DS18B20)");
          break;
        default:
          break;
      }

      // Add an empty line for visual purpose
      newLine = true;
      #endif
    }
  }
}

void BME280_Update() {
  if (BME280_readFlag) {
    BME280_readFlag = false;
    
    // Read sensor values
    read_BME280(&bme, &BME280_userSettings, &BME280_dataStorage);

    // Print sensor values
    #ifdef DEBUGGING_OVER_SERIAL

    Serial.print("     Ambient temperature = ");      // Print temperature
    Serial.print(BME280_dataStorage.temperature);
    Serial.println(" ºC (BME280)");

    Serial.print("     Relative humidity = ");        // Print humidity
    Serial.print(BME280_dataStorage.humidity);
    Serial.println(" %RH");
      
    Serial.print("     Atmospheric pressure = ");     // Print barometric pressure
    Serial.print(BME280_dataStorage.pressure);
    Serial.println(" hPa");

    // Add an empty line for visual purpose
    newLine = true;
    #endif
  }
}
