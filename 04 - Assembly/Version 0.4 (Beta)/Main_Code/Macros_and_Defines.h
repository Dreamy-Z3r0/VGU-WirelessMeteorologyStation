#include "Libraries.h"


/****************************
 *** Basic communications ***
 ****************************/
// UART
#define DEBUGGING_OVER_SERIAL   // Un-comment to enable printing over Serial / Comment to disable printing over Serial

#ifdef DEBUGGING_OVER_SERIAL
#define BAUD_RATE 9600          // Define baud rate for Serial monitor
#endif

bool newLine = false;


/********************************
 *** SD card's requirement(s) ***
 ********************************/
//#define SD_CS_Pin default_SSEL_Pin


/*******************************
 *** DS32331 Real-time Clock ***
 *******************************/
RTClib myRTC;       // Create an instance to use DS3231 library
int RTC_data[6];    // Date and time data holder: day - month - year - hour - minute - second
bool readRTC_control_flag = false;  // set "true" to request a date-time reading from the RTC

#define Alarm_InputPin PA2


/*******************************************************************
 *** BME280: Temperature - Humidity - Barometric pressure sensor ***
 *******************************************************************/
#define BME_CS_Pin PA4    // SSEL pin for BME280

Adafruit_BME280 bme(BME_CS_Pin);      // Create an instance to use libraries for BME280

BME280_settings BME280_userSettings;  // Create a local holder for sensor settings
BME280_Data BME280_dataStorage;       // Create a local data storage

unsigned long BME280_counter;
bool BME280_readFlag = false;         // set "true" to request a data reading from the BME280 sensor


/***********************************
 *** DS18B20: Temperature sensor ***
 ***********************************/
#define OneWireBus PB3    // Digital pin as the one-wire bus

DS18B20_settings DS18B20_userSettings;  // Create a local holder for sensor settings

float DS18B20_Temperature;        // Holds temperature conversion result
bool DS18B20_readFlag = false;    // set "true" to request a data reading from the DS18B20 sensor 


/******************
 *** Rain gauge ***
 ******************/
#define RainfallPerTip  0.2794  // mm
#define RainGauge_InputPin PA1
