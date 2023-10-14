#ifndef _RTC_H_
#define _RTC_H_

#define SERIAL_MONITOR_OUTPUT
#define CUSTOM_MINUTE_ALARM 1 // Check for alarm every CUSTOM_MINUTE_ALARM minutes

#include <DS3231.h>

enum DateTime_Control {
   day = 1,   month = 2,    year = 4, 
  hour = 8, minute = 16, second = 32
};

enum Alarm_Type {
  NO_ALARM = 0,
  MINUTE_ALARM = 1,
  DAILY_ALARM = 2,
  DUAL_ALARM = 3
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
    DS3231_Control(uint32_t Alarm_InputPin);

    // Initialisation
    void init(void);  // Initial setups for alarms
    void set_DailyAlarm(byte hour = 9, byte minute = 0, byte second = 0);   // Set daily alarm (Default: 9 A.M)
    
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

    // Return alarm status
    Alarm_Type alarm_status(void);
    
    void request_from_RTC(void);
    bool is_RTC_requested(void);
    void clearRTC_request(void);

    void Alarm_Presence_Processing(void);

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
    bool readRTC_control_flag;    // set "true" to request a date-time reading from the RTC

    uint32_t Alarm_InputPin;      // DS33231 SQW pin serving as alarm trigger
    Alarm_Type Runtime_Alarm;
    
    byte DailyAlarm_Time[3];      // Stores  custom daily alarm value
};


extern DS3231_Control RTC_DS3231;


/*******************************************
 *** External interrupt service routines ***
 *******************************************/
void Alarm_Callback(DS3231_Control* RTC_DS3231);


/*** Archived ***/
void request_from_RTC(DS3231_Control* RTC_DS3231);

#endif
