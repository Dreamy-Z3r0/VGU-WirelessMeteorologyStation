#pragma once

// Basic communication protocols
#include <Wire.h>
#include <SPI.h>

// Logger with an SD card
#include "SD.h"

// Real-time clock
#include <DS3231.h>

// Ambient temperature - Relative humidity - Barometric pressure sensor
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

// Custom functions
#include "RTC.h"
#include "Thermometer_Hygrometer_Barometer.h"
