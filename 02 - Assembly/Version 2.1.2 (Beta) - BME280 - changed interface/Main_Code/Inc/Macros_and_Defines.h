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

/***********************************************
 *** SX1278: Long-Range (LoRa) Communication ***
 ***********************************************/
#define LoRa_SPI_MOSI_Pin PB15
#define LoRa_SPI_MISO_Pin PB14
#define LoRa_SPI_SCLK_Pin PB13

#define LoRa_Module_SPI_Clock LORA_DEFAULT_SPI_FREQUENCY

#define LoRa_NCSS_Pin PB12
#define LoRa_RST_Pin  PA8
#define LoRa_IRQ_Pin  PA11

#define LoRa_Project_Frequency (long)433E6

#define LoRa_Project_Spreading_Factor 12
#define LoRa_Project_Signal_Bandwidth 500E3
#define LoRa_Project_Coding_Rate_4_Denominator 8
#define LoRa_Project_Sync_Word 0x12
#define LoRa_Project_Transmission_Power 17


/********************************
 *** SD card's requirement(s) ***
 ********************************/
//#define SD_CS_Pin default_SSEL_Pin


/*******************************
 *** DS32331 Real-time Clock ***
 *******************************/
#define Alarm_InputPin PA15 


/*******************************************************************
 *** BME280: Temperature - Humidity - Barometric pressure sensor ***
 *******************************************************************/
// #define BME_CS_Pin PA4    // SSEL pin for BME280
#define BME280_I2C_ADDR 0x76


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

#endif