#include "Libraries.h"


/****************************
 *** Basic communications ***
 ****************************/
// UART
#define DEBUGGING_OVER_SERIAL

#ifdef DEBUGGING_OVER_SERIAL
#define BAUD_RATE 9600
#endif

// SPI
#define default_MOSI_Pin PB15
#define default_MISO_Pin PB14
#define default_SCLK_Pin PB13
#define default_SSEL_Pin PB12


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


/*******************************************************************
 *** BME280: Temperature - Humidity - Barometric pressure sensor ***
 *******************************************************************/
#define BME_CS_Pin PA4    // SSEL pin for BME280

Adafruit_BME280 bme(BME_CS_Pin);    // Create an instance to use libraries for BME280
BME280_settings BME280_userSettings;
BME280_Data BME280_dataStorage;     // Create a local data storage

#define BME280_starting             // enables a startup period for BME280
bool BME280_readFlag = false;       // set "true" to request a data reading from the BME280 sensor 

unsigned long startup_timestamp;
