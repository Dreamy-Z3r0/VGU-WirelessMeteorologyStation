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
DS3231_Control RTC_DS3231;
