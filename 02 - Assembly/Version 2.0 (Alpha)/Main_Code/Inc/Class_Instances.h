#pragma once

#include "Macros_and_Defines.h"


/*******************************
 *** DS32331 Real-time Clock ***
 *******************************/
DS3231_Control RTC_DS3231;


/*******************************************************************
 *** BME280: Temperature - Humidity - Barometric pressure sensor ***
 *******************************************************************/
Adafruit_BME280 bme280(BME_CS_Pin);    // Create an instance to use libraries for BME280
BME280_Control BME280_Device(&bme280);


/***********************************
 *** DS18B20: Temperature sensor ***
 ***********************************/
DS18B20_Control DS18B20_Device(OneWireBus);


/******************
 *** Rain gauge ***
 ******************/
Precipitation RainGauge(RainGauge_InputPin, Alarm_InputPin);


/*****************
 *** Wind vane ***
 *****************/
//WindVane_Control WindVane;


/******************
 *** Anemometer ***
 ******************/
Anemometer_Control Anemometer_Device(Anemometer_InputPin);


/*********************************
 *** General sensor controller ***
 *********************************/
Sensor_Control Project_Sensor( &Anemometer_Device,
                               &WindVane, 
                               &RainGauge,
                               &BME280_Device,
                               &DS18B20_Device
                             );