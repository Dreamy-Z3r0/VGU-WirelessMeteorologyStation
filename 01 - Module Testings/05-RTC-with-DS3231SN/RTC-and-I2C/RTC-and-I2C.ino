/******************************************************************
 ******************************************************************
 ***   DS3231 RTC test for STM32F103CBT6 on Arduino framework   ***
 *** ---------------------------------------------------------- ***
 ***                                                            ***
 *** I2C is one of the most basic communication interface on    ***
 *** microcontrollers, so it is deal to test it with DS3231,    ***
 *** which offers RTC feature to any projects.                  ***
 ***                                                            ***
 *** On the STM32F103CBT6, the default I2C is mapped to I2C1:   ***
 *** (SDA1, SCL1) = (PB7, PB6).                                 ***
 ***                                                            ***
 *** Hardware timer on the STM32 is also tested. TIM1 to TIM4   ***
 *** are available, and TIM4 is in use. The example for timer   ***
 *** part is Timebase_callback_with_parameter.ino on wiki.      ***
 ***                                                            *** 
 *** The RTC IC in use is the DS3231SN. The Arduino library is  *** 
 *** "DS3231" (version 1.1.0) by Andrew Wickert, Eric Ayars,    ***
 *** Jean-Claude Wippler, Northern Widget LLC (available via    *** 
 *** Arduino IDE's Library Manager), introduced in              ***
 *** https://www.arduino.cc/reference/en/libraries/ds3231/      ***
 ***                                                            ***
 *** Note: DS3231M belongs to the DS3231 family. However, it    ***
 ***       lacks temperature compensation, thus unreliable time ***
 ***       data produced. Therefore, this RTC IC branch may be  ***
 ***       suggested for software testing at early stages only. ***
 ******************************************************************
 ******************************************************************/
 

#include <Wire.h>
#include <DS3231.h>

bool readRTC_control_flag = false;  // true every 1 second to request a date-time reading from the RTC

enum DateTime_Control {
  day = 1, 
  month = 2, 
  year = 4, 
  hour = 8, 
  minute = 16, 
  second = 32
};

RTClib myRTC;
int RTC_data[6];    // Date and time data holder: day - month - year - hour - minute - second

typedef struct {
  int year;
  int month;
  int day;
} Date;   // Temporary storage for input date processing function 

typedef struct {
  int hour;
  int minute;
  int second;
} Time;   // Temporary storage for input time processing function 

void setup() {
  Serial.begin(9600);
  Wire.begin();

  HardwareTimer *fetch_RTC = new HardwareTimer(TIM4);
  fetch_RTC->setOverflow(1, HERTZ_FORMAT);  // callback runs every 1 second
  fetch_RTC->attachInterrupt(std::bind(request_from_RTC, &readRTC_control_flag));
  fetch_RTC->resume();

  readRTC(RTC_data);
}

void loop() {
  if (Serial.available()) {   // Check for user input
    String input = Serial.readStringUntil('\n');
    bool inputVerification;
    
    String input_processed = input.substring(0,5);  // Identifier of input data
    if (input_processed.equals("Date:")) {    // Input is date data
      if (15 != input.length()) {   // Expected input length for DD-MM-YYYY format
        Serial.println("Invalid format. Date should be input in DD-MM-YYYY format.");
      } else {
        input_processed = input.substring(5);   // Extract the data string
        input_DateTime(input_processed, RTC_data, day|month|year, &inputVerification);  // Process the extracted data and verify
        if (inputVerification) update_RTC(RTC_data, day|month|year);  // Update RTC if the extracted data is valid
      }
    } else if (input_processed.equals("Time:")) {   // Input is time data
      if ((13 == input.length()) || (10 == input.length())) {   // Expected input lengths for HH:MM:SS and HH:MM formats
        input_processed = input.substring(5);   // Extract the data string

        if (13 == input.length()) {   // HH:MM:SS format
          input_DateTime(input_processed, RTC_data, hour|minute|second, &inputVerification);  // Process the extracted data and verify
          if (inputVerification) update_RTC(RTC_data, hour|minute|second);  // Update RTC if the extracted data is valid
        } else {    // HH:MM format
          input_DateTime(input_processed, RTC_data, hour|minute, &inputVerification);  // Process the extracted data and verify
          if (inputVerification) update_RTC(RTC_data, hour|minute);  // Update RTC if the extracted data is valid
        }
      } else {
        Serial.println("Invalid format. Time should be input in HH:MM or HH:MM:SS formats.");
      }
    } else {  // Invalid data input
      Serial.print("Unrecogised input identifier: ");
      Serial.println(input.substring(0,4));
    }
  }

  if (readRTC_control_flag) {
    readRTC_control_flag = false;
    
    // Get calendar and time data
    readRTC(RTC_data);

    // Print day
    Serial.print(RTC_data[0] < 10 ? "0" : "\0");
    Serial.print(RTC_data[0]);

    // Print month
    Serial.print(RTC_data[1] < 10 ? "/0" : "/");
    Serial.print(RTC_data[1]);

    // Print year
    Serial.print("/");
    Serial.print(RTC_data[2]);

    // Print hour
    Serial.print(RTC_data[3] < 10 ? " 0" : " ");
    Serial.print(RTC_data[3]);

    // Print minute
    Serial.print(RTC_data[4] < 10 ? ":0" : ":");
    Serial.print(RTC_data[4]);

    // Print second
    Serial.print(RTC_data[5] < 10 ? ":0" : ":");
    Serial.print(RTC_data[5]);

    DS3231 RTC_quality;
    Serial.println(RTC_quality.oscillatorCheck() ? " (ok)" : "!");
  }
}

void readRTC(int* storage) {
  int index = 0;
  DateTime now = myRTC.now();
  
  *(storage + index++) = now.day();
  *(storage + index++) = now.month();
  *(storage + index++) = now.year();

  *(storage + index++) = now.hour();
  *(storage + index++) = now.minute();
  *(storage + index)   = now.second();
}

// Extract the input data and check the validity before storing for updating
void input_DateTime(String input_data, int* storage, int type, bool* entryVerification)
{
  *entryVerification = false;
  
  Serial.print("Data to be processed: ");
  Serial.println(input_data);

  switch (type) {
    case (day|month|year): { // DD-MM-YYYY or DD/MM/YYYY
      Date DateFromInput;
      DateFromInput.year = input_data.substring(6).toInt();
      DateFromInput.month = input_data.substring(3,5).toInt();
      DateFromInput.day = input_data.substring(0,2).toInt();

      *entryVerification = isValidDate(&DateFromInput);
    
      Serial.print("Date: ");
      if (DateFromInput.day < 10) Serial.print("0");
      Serial.print(DateFromInput.day);
      Serial.print("/");
      if (DateFromInput.month < 10) Serial.print("0");
      Serial.print(DateFromInput.month);
      Serial.print("/");
      Serial.print(DateFromInput.year);
      Serial.println(*entryVerification ? " (valid date)" : " (invalid date)");

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

      Serial.print("Time: ");
      if (TimeFromInput.hour < 10) Serial.print("0");
      Serial.print(TimeFromInput.hour);
      Serial.print(":");
      if (TimeFromInput.minute < 10) Serial.print("0");
      Serial.print(TimeFromInput.minute);
      Serial.print(":");
      if (TimeFromInput.second < 10) Serial.print("0");
      Serial.print(TimeFromInput.second);
      Serial.println(*entryVerification ? " (valid time)" : " (invalid time)");

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

      Serial.print("Time: ");
      if (TimeFromInput.hour < 10) Serial.print("0");
      Serial.print(TimeFromInput.hour);
      Serial.print(":");
      if (TimeFromInput.minute < 10) Serial.print("0");
      Serial.print(TimeFromInput.minute);
      Serial.println(*entryVerification ? " (valid time)" : " (invalid time)");

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
      Serial.println("Date updated.");
      break;
    }
    case (hour|minute|second): {  // Update time in full format
      RTC_update_func.setClockMode(false);  // set to 24h
      RTC_update_func.setHour((byte)(*(newData+3)));          // new hour
      RTC_update_func.setMinute((byte)(*(newData+4)));        // new minute
      RTC_update_func.setSecond((byte)(*(newData+5)));        // new second
      Serial.println("Time updated.");
      break;
    }
    case (hour|minute): {   // Update time without second
      RTC_update_func.setClockMode(false);  // set to 24h
      RTC_update_func.setHour((byte)(*(newData+3)));          // new hour
      RTC_update_func.setMinute((byte)(*(newData+4)));        // new minute
      Serial.println("Time updated.");
      break;
    }
    default:  // Default case never occurs
      break;
  }
}

void request_from_RTC(bool* controlFlag) {
  *controlFlag = true;
}
