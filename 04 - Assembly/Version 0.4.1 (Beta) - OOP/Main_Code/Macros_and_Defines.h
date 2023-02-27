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
#define Alarm_InputPin PB12
DS3231_Control RTC_DS3231;


/*******************************************************************
 *** BME280: Temperature - Humidity - Barometric pressure sensor ***
 *******************************************************************/
#define BME_CS_Pin PA4    // SSEL pin for BME280

Adafruit_BME280 bme280(BME_CS_Pin);    // Create an instance to use libraries for BME280
BME280_Control BME280_Device(&bme280);


/***********************************
 *** DS18B20: Temperature sensor ***
 ***********************************/
#define OneWireBus PB3    // Digital pin as the one-wire bus
DS18B20_Control DS18B20_Device(OneWireBus);


/******************
 *** Rain gauge ***
 ******************/
#define RainGauge_InputPin PA1
Precipitation RainGauge;
