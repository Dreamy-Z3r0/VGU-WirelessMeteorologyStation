#ifndef _MACROS_AND_DEFINES_H_
#define _MACROS_AND_DEFINES_H_


/****************************
 *** Basic communications ***
 ****************************/
// UART
// #define DEBUGGING_OVER_SERIAL   // Un-comment/Comment to enable/disable printing over Serial for debugging
#define SERIAL_MONITOR_OUTPUT   // Un-comment/Comment to enable/disable printing messages on Serial Monitor
// #define RASPBERRY_PI_LOGGING

#define BAUD_RATE 9600          // Define baud rate for Serial monitor


/*******************************
 *** DS32331 Real-time Clock ***
 *******************************/
// #define Alarm_InputPin PA15   -> must be directly declared in RTC.cpp


/*******************************************************************
 *** BME280: Temperature - Humidity - Barometric pressure sensor ***
 *******************************************************************/
#define BME280_I2C_ADDR 0x76


/***********************************
 *** DS18B20: Temperature sensor ***
 ***********************************/
#define OneWireBus PB3    // Digital pin as the one-wire bus

#endif