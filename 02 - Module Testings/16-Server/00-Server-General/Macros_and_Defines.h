#ifndef _MACROS_AND_DEFINES_H_
#define _MACROS_AND_DEFINES_H_

#include "Libraries.h"


/****************************
 *** Basic communications ***
 ****************************/
// UART
// #define DEBUGGING_OVER_SERIAL   // Un-comment/Comment to enable/disable printing over Serial for debugging
#define SERIAL_MONITOR_OUTPUT   // Un-comment/Comment to enable/disable printing messages on Serial Monitor
// #define RASPBERRY_PI_LOGGING

#if defined(DEBUGGING_OVER_SERIAL)
    #define BAUD_RATE 9600          // Define baud rate for Serial monitor
#elif defined(SERIAL_MONITOR_OUTPUT)
    #define BAUD_RATE 9600          // Define baud rate for Serial monitor
#elif defined(RASPBERRY_PI_LOGGING)
    #define BAUD_RATE 9600          // Define baud rate for communication with Raspberry Pi
#endif


/********************************
 *** SD card's requirement(s) ***
 ********************************/
#define MICROSD_SPI1_MOSI_Pin 13
#define MICROSD_SPI1_MISO_Pin 12
#define MICROSD_SPI1_SCLK_Pin 14
#define MICROSD_SPI1_NCSS_Pin 15


/*******************************
 *** DS32331 Real-time Clock ***
 *******************************/
#define Alarm_InputPin 0


/***********************************
 *** DS18B20: Temperature sensor ***
 ***********************************/
#define OneWireBus 16    // Digital pin as the one-wire bus


#endif
