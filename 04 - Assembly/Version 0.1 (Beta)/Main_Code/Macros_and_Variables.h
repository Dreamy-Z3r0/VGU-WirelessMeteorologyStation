#include "Libraries.h"


/****************************
 *** Basic communications ***
 ****************************/
// UART
#define DEBUGGING_OVER_SERIAL

#ifdef DEBUGGING_OVER_SERIAL
#define BAUD_RATE 9600
#endif


/*******************************
 *** DS32331 Real-time Clock ***
 *******************************/
RTClib myRTC;       // Create an instance to use DS3231 library
int RTC_data[6];    // Date and time data holder: day - month - year - hour - minute - second
bool readRTC_control_flag = false;  // set "true" to request a date-time reading from the RTC
