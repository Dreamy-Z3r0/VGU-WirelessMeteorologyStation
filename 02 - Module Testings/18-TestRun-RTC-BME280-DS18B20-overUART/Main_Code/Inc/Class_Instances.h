#ifndef _CLASS_INSTANCES_H_
#define _CLASS_INSTANCES_H_

#include "Libraries.h"
#include "Macros_and_Defines.h"


/*******************************
 *** DS32331 Real-time Clock ***
 *******************************/
// DS3231_Control RTC_DS3231;   <--- Defined within RTC.cpp for global access


/*******************************************************************
 *** BME280: Temperature - Humidity - Barometric pressure sensor ***
 *******************************************************************/
Adafruit_BME280 bme280;    // Create an instance to use libraries for BME280
BME280_Control BME280_Device(&bme280);


/***********************************
 *** DS18B20: Temperature sensor ***
 ***********************************/
DS18B20_Control DS18B20_Device(OneWireBus);


/*********************************
 *** General sensor controller ***
 *********************************/
Sensor_Control Project_Sensor( &BME280_Device,
                               &DS18B20_Device );
                            
#endif