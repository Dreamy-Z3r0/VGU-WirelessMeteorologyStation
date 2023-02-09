#ifndef _TF_LOCALSTORAGE_H_
#define _TF_LOCALSTORAGE_H_

#include "Libraries.h"
#include "Macros_and_Defines.h"

#include "RTC.h"

#define FILENAME_SEPARATOR "-"
#define FILENAME_EXTENSION ".csv"

#define SYSTEM_FILE "LOCAL-STORAGE.csv"


class Card_Access {
    public:
        Card_Access(uint32_t CSPin);

        void init();

        bool create_daily_log(void);

        bool log_entry(String data);

    private:
        uint32_t CSPin;
        bool cardAvailable, calendarAvailable;

        int RTC_data[3];    // Year - Month - Day
        String DailyLog_Name = "";
  
        bool update_calendar(void);
        void generate_dailylog_filename(void);
};


#endif