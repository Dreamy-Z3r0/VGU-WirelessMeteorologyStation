#ifndef _LIBRARIES_H_
#define _LIBRARIES_H_


/****************************************************
 *** Built-in and community-contributed libraries ***
 ****************************************************/

#include <Arduino.h>

// Basic communication protocols
#include <Wire.h>

// Real-time clock DS3231
#include <DS3231.h>

// Temperature - Humidity - Barometric pressure sensor BME280
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

// Temperature sensor DS18B20
#include <OneWire.h>


/**********************
 *** Custom classes ***
 **********************/

// Time handler
#include "RTC.h"

// Sensor configuration and control
#include "SensorUnits/Sensor_Base.h"
#include "SensorUnits/Thermometer_Hygrometer_Barometer.h"
#include "Sensor_Control.h"

#endif