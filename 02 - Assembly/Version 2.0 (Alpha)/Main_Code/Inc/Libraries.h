#pragma once

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

// Custom classes
#include "../Inc/RTC.h"

#include "Sensor_Base.h"

#include "../Inc/SensorUnits/Thermometer_Hygrometer_Barometer.h"
#include "../Inc/SensorUnits/Precipitation.h"
#include "../Inc/SensorUnits/Wind_Vane.h"
#include "../Inc/SensorUnits/Anemometer.h"

#include "Sensor_Control.h"
