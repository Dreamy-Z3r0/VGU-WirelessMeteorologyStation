#include "Libraries.h"


/****************************
 *** Basic communications ***
 ****************************/
// UART
#define DEBUGGING_OVER_SERIAL

#ifdef DEBUGGING_OVER_SERIAL
#define BAUD_RATE 9600
#endif


/********************************
 *** SD card's requirement(s) ***
 ********************************/
//#define SD_CS_Pin default_SSEL_Pin


/*******************************
 *** DS32331 Real-time Clock ***
 *******************************/
DS3231_Control RTC_DS3231;


/*******************************************************************
 *** BME280: Temperature - Humidity - Barometric pressure sensor ***
 *******************************************************************/
#define BME_CS_Pin PA4    // SSEL pin for BME280

Adafruit_BME280 bme280(BME_CS_Pin);    // Create an instance to use libraries for BME280
BME280_Control BME280_Device(&bme280);
