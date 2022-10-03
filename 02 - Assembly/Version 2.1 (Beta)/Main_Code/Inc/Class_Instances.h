#pragma once

#include "Libraries.h"
#include "Macros_and_Defines.h"


/****************************
 *** SPI2 for SX1278 LoRa ***
 ****************************/
// SPIClass SPI_2(LoRa_SPI_MOSI_Pin, LoRa_SPI_MISO_Pin, LoRa_SPI_SCLK_Pin);


/*******************************
 *** DS32331 Real-time Clock ***
 *******************************/
// DS3231_Control RTC_DS3231;   <--- Defined within RTC.cpp for global access


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
//WindVane_Control WindVane;   <--- Defined within Wind_Vane.cpp for the use of HAL library


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