#include "../Inc/RTC.h"


DS3231_Control RTC_DS3231(Alarm_InputPin);

uint8_t counter = 0;


/*********************
 *** Contructor(s) ***
 *********************/

DS3231_Control::DS3231_Control() {
  readRTC_control_flag = false;
}

DS3231_Control::DS3231_Control(uint32_t Alarm_InputPin) {
  this->Alarm_InputPin = Alarm_InputPin;
  readRTC_control_flag = false;
}


/*****************************
 *** Device initialisation ***
 *****************************/

// Initial setups for alarms
void DS3231_Control::init(void) {
  pinMode(Alarm_InputPin, INPUT_PULLUP);    // Initialise alarm pin

  DS3231 clock;
  clock.setA1Time(1, DailyAlarm_Time[0], DailyAlarm_Time[1], DailyAlarm_Time[2], 0x8, false, false, false);    // Set up daily alarm
  clock.setA2Time(1, 0, 0, 0x7, false, false, false);   // Alarm every minute
  
  clock.turnOnAlarm(1);     // Enable alarms
  clock.turnOnAlarm(2);

  clock.checkIfAlarm(1);    // Make sure there are no alarms present initially
  clock.checkIfAlarm(2);

  attachInterrupt(digitalPinToInterrupt(Alarm_InputPin), std::bind(Alarm_Callback, this), LOW);    // Enable interrupt from alarm
}

// Set daily alarm (Default: 9 A.M)
void DS3231_Control::set_DailyAlarm(byte hour, byte minute, byte second) {
  DailyAlarm_Time[0] = hour;
  DailyAlarm_Time[1] = minute;
  DailyAlarm_Time[2] = second;

  DS3231 clock;
  clock.turnOffAlarm(1);    // Disable alarms
  clock.turnOffAlarm(2);
  clock.setA1Time(1, DailyAlarm_Time[0], DailyAlarm_Time[1], DailyAlarm_Time[2], 0x8, false, false, false);    // Set up new daily alarm
  clock.turnOnAlarm(1);     // Enable alarms
  clock.turnOnAlarm(2);

  clock.checkIfAlarm(1);    // Make sure there are no alarms present initially
  clock.checkIfAlarm(2);
}


// Fetch time kept by the DS3231
void DS3231_Control::readRTC(void) {
  uint8_t index = 0;
  DateTime now = myRTC.now();

  RTC_data[index++] = now.day();
  RTC_data[index++] = now.month();
  RTC_data[index++] = now.year();

  RTC_data[index++] = now.hour();
  RTC_data[index++] = now.minute();
  RTC_data[index]   = now.second();
}

int DS3231_Control::readDay(void) {
  return RTC_data[0];
}

int DS3231_Control::readMonth(void) {
  return RTC_data[1];
}

int DS3231_Control::readYear(void) {
  return RTC_data[2];
}

int DS3231_Control::readHour(void) {
  return RTC_data[3];
}

int DS3231_Control::readMinute(void) {
  return RTC_data[4];
}

int DS3231_Control::readSecond(void) {
  return RTC_data[5];
}

// Process input data for date/time as String
void DS3231_Control::DateTime_InputString_Processing(String input_data) {
  bool inputVerification;

  String input_processed = input_data.substring(0, input_data.indexOf(':'));  // Identifier of input data
  if (input_processed.equals("Date")) {    // Input is date data
    if (15 != input_data.length()) {   // Expected input length for DD-MM-YYYY format
      #ifdef SERIAL_MONITOR_OUTPUT
      Serial.println("Invalid format. Date should be input in DD-MM-YYYY format.");
      #endif
    } else {
      input_processed = input_data.substring(5);   // Extract the data string
      input_DateTime(input_processed, day|month|year, &inputVerification);  // Process the extracted data and verify
      if (inputVerification) update_RTC(day|month|year);  // Update RTC if the extracted data is valid
    }
  } else if (input_processed.equals("Time")) {   // Input is time data
    if ((13 == input_data.length()) || (10 == input_data.length())) {   // Expected input lengths for HH:MM:SS and HH:MM formats
      input_processed = input_data.substring(5);   // Extract the data string

      if (13 == input_data.length()) {   // HH:MM:SS format
        input_DateTime(input_processed, hour|minute|second, &inputVerification);  // Process the extracted data and verify
        if (inputVerification) update_RTC(hour|minute|second);  // Update RTC if the extracted data is valid
      } else {    // HH:MM format
        input_DateTime(input_processed, hour|minute, &inputVerification);  // Process the extracted data and verify
        if (inputVerification) update_RTC(hour|minute);  // Update RTC if the extracted data is valid
      }
    } else {
      #ifdef SERIAL_MONITOR_OUTPUT
      Serial.println("Invalid format. Time should be input in HH:MM or HH:MM:SS formats.");
      #endif
    }
  } else {  // Invalid data input
    #ifdef SERIAL_MONITOR_OUTPUT
    Serial.printf("Unrecogised input identifier: %s\n", input_data.substring(0,4));
    #endif
  }
}

// Extract the input data and check the validity before storing for updating
void DS3231_Control::input_DateTime(String input_data, int type, bool* entryVerification) {
  *entryVerification = false;

  switch (type) {
    case (day|month|year): { // DD-MM-YYYY or DD/MM/YYYY
      Date DateFromInput;
      DateFromInput.year = input_data.substring(6).toInt();
      DateFromInput.month = input_data.substring(3, 5).toInt();
      DateFromInput.day = input_data.substring(0, 2).toInt();

      *entryVerification = isValidDate(&DateFromInput);

      if (*entryVerification) {
        RTC_data[0] = DateFromInput.day;
        RTC_data[1] = DateFromInput.month;
        RTC_data[2] = DateFromInput.year;
      }
      break;
    }
    case (hour|minute|second): {
      Time TimeFromInput;
      TimeFromInput.hour = input_data.substring(0, 2).toInt();
      TimeFromInput.minute = input_data.substring(3, 5).toInt();
      TimeFromInput.second = input_data.substring(6, 8).toInt();

      *entryVerification = isValidTime(&TimeFromInput);

      if (*entryVerification) {
        RTC_data[3] = TimeFromInput.hour;
        RTC_data[4] = TimeFromInput.minute;
        RTC_data[5] = TimeFromInput.second;
      }
      break;
    }
    case (hour|minute): {
      Time TimeFromInput;
      TimeFromInput.hour = input_data.substring(0, 2).toInt();
      TimeFromInput.minute = input_data.substring(3, 5).toInt();
      TimeFromInput.second = 0;

      *entryVerification = isValidTime(&TimeFromInput);

      if (*entryVerification) {
        RTC_data[3] = TimeFromInput.hour;
        RTC_data[4] = TimeFromInput.minute;
      }
      break;
    }
    default:
      break;
  }
}

// Validate input date
bool DS3231_Control::isValidDate(Date* inputDate) {
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
bool DS3231_Control::isValidTime(Time* inputTime) {   // Check the validity of input time
  if (  (0 > inputTime->hour) | (23 < inputTime->hour)  ) return false;   // 0 <= hour <= 23
  if ((0 > inputTime->minute) | (59 < inputTime->minute)) return false;   // 0 <= minute <= 59
  if ((0 > inputTime->second) | (59 < inputTime->second)) return false;   // 0 <= second <= 59

  return true;
}

// Update the RTC
void DS3231_Control::update_RTC(int type) {
  DS3231 RTC_update_func;   // DS3231 instance to access set* functions

  switch (type) {
    case (day|month|year): {  // Update RTC date
      RTC_update_func.setYear((RTC_data[2]) % 100);  // new year
      RTC_update_func.setMonth(RTC_data[1]);        // new month
      RTC_update_func.setDate(RTC_data[0]);         // new date

      #ifdef SERIAL_MONITOR_OUTPUT
      Serial.println("Date updated.");
      #endif
      
      break;
    }
    case (hour|minute|second): {  // Update time in full format
      RTC_update_func.setClockMode(false);  // set to 24h
      RTC_update_func.setHour(RTC_data[3]);        // new hour
      RTC_update_func.setMinute(RTC_data[4]);      // new minute
      RTC_update_func.setSecond(RTC_data[5]);      // new second

      #ifdef SERIAL_MONITOR_OUTPUT
      Serial.println("Time updated.");
      #endif

      break;
    }
    case (hour|minute): {   // Update time without second
      RTC_update_func.setClockMode(false);  // set to 24h
      RTC_update_func.setHour(RTC_data[3]);        // new hour
      RTC_update_func.setMinute(RTC_data[4]);      // new minute

      #ifdef SERIAL_MONITOR_OUTPUT
      Serial.println("Time updated.");
      #endif
        
      break;
    }
    default:  // Default case never occurs
      break;
  }
}

void DS3231_Control::alarm_status(void) {
  return Runtime_Alarm;
}

void DS3231_Control::request_from_RTC(void) {
  readRTC_control_flag = true;
}

bool DS3231_Control::is_RTC_requested(void) {
  return readRTC_control_flag;
}

void DS3231_Control::clearRTC_request(void) {
  readRTC_control_flag = false;
}


/*******************************************
 *** External interrupt service routines ***
 *******************************************/

void DS3231_Control::Alarm_Presence_Processing(void) {
  DS3231 alarm;

  if (alarm.checkIfAlarm(2)) {    // Check and clear the alarm of Alarm 2 from DS3231
    Runtime_Alarm = MINUTE_ALARM;
  }

  if (alarm.checkIfAlarm(1)) {    // Check and clear the alarm of Alarm 1 from DS3231
    Runtime_Alarm = DAILY_ALARM;
  }
}

void Alarm_Callback(DS3231_Control* RTC_DS3231) {
  RTC_DS3231->Alarm_Presence_Processing();
}


/*** Archived ***/
void request_from_RTC(DS3231_Control* RTC_DS3231) {
  RTC_DS3231->request_from_RTC();
}
