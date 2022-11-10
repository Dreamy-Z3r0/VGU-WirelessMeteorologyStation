#ifndef _RTC_H_
#define _RTC_H_

#include <DS3231.h>

enum DateTime_Control {
   day = 1,   month = 2,    year = 4, 
  hour = 8, minute = 16, second = 32
};

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


class DS3231_Control {  
  public:    
    DS3231_Control();
  
    // Fetch time kept by the DS3231
    void readRTC(void);
    // Process String input data for date/time
    void DateTime_InputString_Processing(String input_data);

    // Return date data
    int readDay(void);    // Day of month
    int readMonth(void);  // Month
    int readYear(void);   // Year

    // Return time data
    int readHour(void);     // Hour
    int readMinute(void);   // Minute
    int readSecond(void);   // Second
    
    void request_from_RTC(void);
    bool is_RTC_requested(void);
    void clearRTC_request(void);

  protected:
    // Update the RTC
    void update_RTC(int type);

    // Extract the input data and check the validity before storing for updating
    void input_DateTime(String input_data, int type, bool* entryVerification);

    // Validate input date
    bool isValidDate(Date* inputDate);
    // Validate input time
    bool isValidTime(Time* inputTime);

  private:
    RTClib myRTC;
    int RTC_data[6];    // Date and time data holder: day - month - year - hour - minute - second
    bool readRTC_control_flag;      // set "true" to request a date-time reading from the RTC
};


extern DS3231_Control RTC_DS3231;

void request_from_RTC(DS3231_Control* RTC_DS3231);

#endif
