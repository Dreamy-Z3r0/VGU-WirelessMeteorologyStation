#pragma once


/****************************************************
 *** Built-in and community-contributed libraries ***
 ****************************************************/

// Basic communication protocols
#include <Wire.h>
#include <SPI.h>

// Logger with an SD card
#include <SD.h>

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

#include "SensorUnits/Anemometer.h"
#include "SensorUnits/Wind_Vane.h"
#include "SensorUnits/Precipitation.h"
#include "SensorUnits/Thermometer_Hygrometer_Barometer.h"

#include "Sensor_Control.h"
