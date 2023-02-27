#include "RTC.h"


// Fetch time kept by the DS3231
void readRTC(RTClib* myRTC, int* storage) {
  int index = 0;
  DateTime now = myRTC->now();
  
  *(storage + index++) = now.day();
  *(storage + index++) = now.month();
  *(storage + index++) = now.year();

  *(storage + index++) = now.hour();
  *(storage + index++) = now.minute();
  *(storage + index)   = now.second();
}

// Extract the input data and check the validity before storing for updating
void input_DateTime(String input_data, int* storage, int type, bool* entryVerification) {
  *entryVerification = false;

  switch (type) {
    case (day|month|year): { // DD-MM-YYYY or DD/MM/YYYY
      Date DateFromInput;
      DateFromInput.year = input_data.substring(6).toInt();
      DateFromInput.month = input_data.substring(3,5).toInt();
      DateFromInput.day = input_data.substring(0,2).toInt();

      *entryVerification = isValidDate(&DateFromInput);

      if (*entryVerification) {
        *storage = DateFromInput.day;
        *(storage + 1) = DateFromInput.month;
        *(storage + 2) = DateFromInput.year;
      }
      break;
    }
    case (hour|minute|second): {
      Time TimeFromInput;
      TimeFromInput.hour = input_data.substring(0,2).toInt();
      TimeFromInput.minute = input_data.substring(3,5).toInt();
      TimeFromInput.second = input_data.substring(6,8).toInt();

      *entryVerification = isValidTime(&TimeFromInput);

      if (*entryVerification) {
        *(storage + 3) = TimeFromInput.hour;
        *(storage + 4) = TimeFromInput.minute;
        *(storage + 5) = TimeFromInput.second;
      }
      break;
    }
    case (hour|minute): {
      Time TimeFromInput;
      TimeFromInput.hour = input_data.substring(0,2).toInt();
      TimeFromInput.minute = input_data.substring(3,5).toInt();
      TimeFromInput.second = 0;

      *entryVerification = isValidTime(&TimeFromInput);

      if (*entryVerification) {
        *(storage + 3) = TimeFromInput.hour;
        *(storage + 4) = TimeFromInput.minute;
      }
      break;
    }
    default:
      break;
  }
}

// Validate input date
bool isValidDate(Date* inputDate) {
  if (inputDate->year < 0) return false;  // year must be a non-negative value
  
  if (2 == inputDate->month) {    // Check the day of February: 1-28 for common years and 1-29 for leap years
    return ((1 <= inputDate->day) && ((isleapYear(inputDate->year) ? 29 : 28) >= inputDate->day));
  } else if ((7 >= inputDate->month) && (1 <= inputDate->month)) {    // Check the day of months from January to July, excluding February
    return ((1 <= inputDate->day) && (((0 == (inputDate->month % 2)) ? 30 : 31) >= inputDate->day));
  } else if ((8 <= inputDate->month) && (12 >= inputDate->month)) {   // Check the day of months from August to December
    return ((1 <= inputDate->day) && (((0 == (inputDate->month % 2)) ? 31 : 30) >= inputDate->day));
  } else {    // This case occurs when the month input is out of range [1, 12]
    return false;
  }
}

// Validate input time
bool isValidTime(Time* inputTime) {   // Check the validity of input time
  if ((0 > inputTime->hour) | (23 < inputTime->hour)) return false;       // 0 <= hour <= 23
  if ((0 > inputTime->minute) | (59 < inputTime->minute)) return false;   // 0 <= minute <= 59
  if ((0 > inputTime->second) | (59 < inputTime->second)) return false;   // 0 <= second <= 59

  return true;
}
  
// Update the RTC
void update_RTC(int* newData, int type) {
  DS3231 RTC_update_func;   // DS3231 instance to access set* functions

  switch (type) {
    case (day|month|year): {  // Update RTC date
      RTC_update_func.setYear((byte)(*(newData+2) % 100));    // new year
      RTC_update_func.setMonth((byte)(*(newData+1)));         // new month
      RTC_update_func.setDate((byte)(*newData));              // new date
      #ifdef DEBUGGING_OVER_SERIAL
      Serial.println("Date updated.");
      #endif
      break;
    }
    case (hour|minute|second): {  // Update time in full format
      RTC_update_func.setClockMode(false);  // set to 24h
      RTC_update_func.setHour((byte)(*(newData+3)));          // new hour
      RTC_update_func.setMinute((byte)(*(newData+4)));        // new minute
      RTC_update_func.setSecond((byte)(*(newData+5)));        // new second
      #ifdef DEBUGGING_OVER_SERIAL
      Serial.println("Time updated.");
      #endif
      break;
    }
    case (hour|minute): {   // Update time without second
      RTC_update_func.setClockMode(false);  // set to 24h
      RTC_update_func.setHour((byte)(*(newData+3)));          // new hour
      RTC_update_func.setMinute((byte)(*(newData+4)));        // new minute
      #ifdef DEBUGGING_OVER_SERIAL
      Serial.println("Time updated.");
      #endif
      break;
    }
    default:  // Default case never occurs
      break;
  }
}

void request_from_RTC(bool* controlFlag) {
  *controlFlag = true;
}
