#include <DS3231.h>


enum DateTime_Control {
  day = 1, 
  month = 2, 
  year = 4, 
  hour = 8, 
  minute = 16, 
  second = 32
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


// Fetch time kept by the DS3231
void readRTC(RTClib* myRTC, int* storage);
// Extract the input data and check the validity before storing for updating
void input_DateTime(String input_data, int* storage, int type, bool* entryVerification);
// Validate input date
bool isValidDate(Date* inputDate);
// Validate input time
bool isValidTime(Time* inputTime);
// Update the RTC
void update_RTC(int* newData, int type);

void request_from_RTC(bool* controlFlag);
