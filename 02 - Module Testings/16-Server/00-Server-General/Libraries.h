#ifndef _LIBRARIES_H_
#define _LIBRARIES_H_


/****************************************************
 *** Built-in and community-contributed libraries ***
 ****************************************************/

#include <Arduino.h>

// Basic communication protocols
#include <Wire.h>
#include <SPI.h>

// Logger with an SD card
#include <SD.h>

// Real-time clock DS3231
#include <DS3231.h>

// Temperature sensor DS18B20
#include <OneWire.h>


/**********************
 *** Custom classes ***
 **********************/

// Time handler
#include "RTC_DS3231.h"

// Sensor configuration and control
#include "Sensor_Base.h"

#include "Thermometer.h"


#endif
