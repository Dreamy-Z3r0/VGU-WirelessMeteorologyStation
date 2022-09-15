#include "Inc/Libraries.h"
#include "Inc/Macros_and_Defines.h"

bool Print_WindSpeed = false;

void setup() {
  #ifdef DEBUGGING_OVER_SERIAL
  Serial.begin(BAUD_RATE);
  #endif

  Wire.begin();
  Project_Sensor.init();

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
    
    if (!Anemometer_Device.is_readFlag_set() && Anemometer_Device.is_standbyFlag_set()) {
      Anemometer_Device.set_readFlag();
    }

    if (!WindVane.is_readFlag_set() && WindVane.is_standbyFlag_set()) {
      WindVane.set_readFlag();
    }
    
    DateTime_Display();
    Print_WindSpeed = true;
  }
  
  if (RainGauge.is_DailyAlarm_available()) {
    RainGauge.clear_DailyAlarm();

    #ifdef DEBUGGING_OVER_SERIAL
    float Rainfall_amount;
    RainGauge.read_sensor_data(&Rainfall_amount);
    Serial.printf("Daily alarm! Precipitation (rainfall): %.4f mm\n", Rainfall_amount);
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

    // Add an empty line for visual purpose
    newLine = true;
    #endif
  }
}

void DS18B20_Update() {
  if (DS18B20_Device.is_readFlag_set()) {
    DS18B20_Device.clear_readFlag();   // Clear the request

    uint8_t present = 0;    // Guards the one-wire bus; if present == 0, no further action shall be taken within this scope.
    DS18B20_Device.update_sensor_data(&present);  // Gets sensor readings

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
      float Sensor_Data;
      DS18B20_Device.read_sensor_data(&Sensor_Data);
      
      Serial.print("     Ambient temperature = ");
      Serial.print(Sensor_Data, present);

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
    BME280_Device.update_sensor_data();

    // Print sensor values
    #ifdef DEBUGGING_OVER_SERIAL

    float *Sensor_Data = new float[3];   // Temperature - Pressure - Humidity
    BME280_Device.read_sensor_data(Sensor_Data);

    // Print temperature
    Serial.printf("     Ambient temperature = %.2f ºC (BME280)\n", *Sensor_Data);    

    // Print humidity  
    Serial.printf("     Relative humidity = %.2f %%RH\n", *(Sensor_Data+2));       

    // Print barometric pressure 
    Serial.printf("     Atmospheric pressure = %.2f hPa\n", *(Sensor_Data+1)); 

    delete[] Sensor_Data;

    // Add an empty line for visual purpose
    newLine = true;
    #endif
  }
}

void WindVane_Update() {
//  if (WindVane.is_readFlag_set()) {
    WindVane.update_sensor_data();

    if (Print_WindSpeed) {
      
      // Print sensor values
      #ifdef DEBUGGING_OVER_SERIAL

      float wind_direction;
      WindVane.read_sensor_data(&wind_direction);
      
      Serial.printf("     Wind direction = %.1f°\n", wind_direction);  // Print wind direction

      // Add an empty line for visual purpose
      newLine = true;
      #endif
    }
}

void Anemometer_Update() {
  Anemometer_Device.update_sensor_data();

  if (Print_WindSpeed) {
    Print_WindSpeed = false;

    #ifdef DEBUGGING_OVER_SERIAL

    float windSpeed;
    Anemometer_Device.read_sensor_data(&windSpeed);
    
    Serial.printf("     Wind speed = %.2f km/h\n", windSpeed);      // Print wind speed   
  
    // Add an empty line for visual purpose
    newLine = true;
  
    #endif
  }
}
