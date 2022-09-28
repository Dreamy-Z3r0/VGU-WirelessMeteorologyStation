#pragma once

#include "Libraries.h"


/****************************
 *** Basic communications ***
 ****************************/
// UART
#define DEBUGGING_OVER_SERIAL   // Un-comment to enable printing over Serial / Comment to disable printing over Serial

#ifdef DEBUGGING_OVER_SERIAL
#define BAUD_RATE 9600          // Define baud rate for Serial monitor
#endif


/********************************
 *** SD card's requirement(s) ***
 ********************************/
//#define SD_CS_Pin default_SSEL_Pin


/*******************************
 *** DS32331 Real-time Clock ***
 *******************************/
#define Alarm_InputPin PB12


/*******************************************************************
 *** BME280: Temperature - Humidity - Barometric pressure sensor ***
 *******************************************************************/
#define BME_CS_Pin PA4    // SSEL pin for BME280


/***********************************
 *** DS18B20: Temperature sensor ***
 ***********************************/
#define OneWireBus PB3    // Digital pin as the one-wire bus


/******************
 *** Rain gauge ***
 ******************/
#define RainGauge_InputPin PA1


/*****************
 *** Wind vane ***
 *****************/
//#define WindVane_InputPin PB1


/******************
 *** Anemometer ***
 ******************/
#define Anemometer_InputPin PA3
// Anemometer_Control Anemometer_Device(Anemometer_InputPin);