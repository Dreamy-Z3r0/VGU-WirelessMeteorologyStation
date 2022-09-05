#include "Libraries.h"
#include "Macros_and_Defines.h"

bool Print_WindSpeed = false;

void setup() {
  #ifdef DEBUGGING_OVER_SERIAL
  Serial.begin(BAUD_RATE);
  #endif

  Wire.begin();
  WindVane.init();
  RainGauge.init(RainGauge_InputPin, Alarm_InputPin);
  Anemometer_Device.init();
//  RainGauge.set_DailyAlarm(12, 14, 45);

  delay(2);
  if (!bme280.begin()) {
    #ifdef DEBUGGING_OVER_SERIAL
    Serial.println("Problem initializing BME280.");
    #endif
    return;
  }
  delay(20);

  BME280_Device.new_SensorMode(Adafruit_BME280::MODE_FORCED);
  BME280_Device.new_TemperatureOversampling(Adafruit_BME280::SAMPLING_X2);
  BME280_Device.new_PressureOversampling(Adafruit_BME280::SAMPLING_X16);
  BME280_Device.new_HumidityOversampling(Adafruit_BME280::SAMPLING_X1);
  BME280_Device.new_FilterCoefficient(Adafruit_BME280::FILTER_X16);
  
  BME280_Device.update_BME280_settings();

  DS18B20_Device.update_DS18B20_settings(R_10BIT);

  HardwareTimer *fetch_RTC = new HardwareTimer(TIM4);
  fetch_RTC->setOverflow(1, HERTZ_FORMAT);  // callback runs every 1 second
  fetch_RTC->attachInterrupt(std::bind(request_from_RTC, &RTC_DS3231));
  fetch_RTC->resume();
}

void loop() {
  DateTime_InputHandler();
  
  if (RTC_DS3231.is_RTC_requested()) {
    DS18B20_Device.set_readFlag();
    BME280_Device.set_readFlag();
    WindVane.set_readFlag();
    
    if (!Anemometer_Device.is_readFlag_set() && Anemometer_Device.is_idlePeriod()) {
      Anemometer_Device.set_readFlag();
    }
    
    DateTime_Display();
    Print_WindSpeed = true;
  }
  
  if (RainGauge.is_DailyAlarm_available()) {
    RainGauge.clear_DailyAlarm();

    #ifdef DEBUGGING_OVER_SERIAL
    Serial.printf("Daily alarm! Precipitation (rainfall): %.4f mm\n", RainGauge.get_Rainfall_Data());
    #endif
  }

  DS18B20_Update();
  BME280_Update();
  WindVane_Update();
  Anemometer_Update();
    
  if (newLine) {
    Serial.println();
    newLine = false;
  }
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
      Serial.printf("Unrecogised input identifier: %s\n", input.substring(0,4));
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

    // Add an empty line for visual purpose
    newLine = true;
    #endif
  }
}

void DS18B20_Update() {
  if (DS18B20_Device.is_readFlag_set()) {
    DS18B20_Device.clear_readFlag();   // Clear the request

    uint8_t present = 0;    // Guards the one-wire bus; if present == 0, no further action shall be taken within this scope.
    DS18B20_Device.convert_Temperature(&present);  // Gets sensor readings

    if (0 == present) {
      #ifdef DEBUGGING_OVER_SERIAL
      Serial.println("     (No device present on the One-Wire bus)");
      #endif
    } else {
      #ifdef DEBUGGING_OVER_SERIAL

      // Recycle the "present" variable for printing valid number of floating point values
      PRECISION thermometerResolution = DS18B20_Device.get_thermometerResolution();
      present = (uint8_t)(((thermometerResolution >> 4) + 1) / 2); 

      // Print temperature
      Serial.print("     Ambient temperature = ");
      Serial.print(DS18B20_Device.get_Temperature(), present);

      switch (thermometerResolution) {
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
  if (BME280_Device.is_readFlag_set()) {
    BME280_Device.clear_readFlag();
    BME280_Device.read_BME280();

    // Print sensor values
    #ifdef DEBUGGING_OVER_SERIAL

    // Print temperature
    Serial.printf("     Ambient temperature = %.2f ºC (BME280)\n", BME280_Device.get_Temperature());    

    // Print humidity  
    Serial.printf("     Relative humidity = %.2f %%RH\n", BME280_Device.get_Humidity());       

    // Print barometric pressure 
    Serial.printf("     Atmospheric pressure = %.2f hPa\n", BME280_Device.get_Pressure());     

    // Add an empty line for visual purpose
    newLine = true;
    #endif
  }
}

void WindVane_Update() {
  if (WindVane.is_readFlag_set()) {
    WindVane.Wind_Direction_Reading_Routine();

    if (WindVane.is_Data_Ready()) {
      WindVane.clear_readFlag();
      
      // Print sensor values
      #ifdef DEBUGGING_OVER_SERIAL
   
      Serial.printf("     Wind direction = %.1f°\n", WindVane.read_Wind_Direction());  // Print wind direction

      // Add an empty line for visual purpose
      newLine = true;
      #endif
    }
  }
}

void Anemometer_Update() {
  Anemometer_Device.Anemometer_Reading_Routine();

  if (Print_WindSpeed) {
    Print_WindSpeed = false;

    #ifdef DEBUGGING_OVER_SERIAL

    Serial.printf("     Wind speed = %.2f km/h\n", Anemometer_Device.read_Wind_Speed());      // Print wind speed
  
    // Add an empty line for visual purpose
    newLine = true;
  
    #endif
  }
}
