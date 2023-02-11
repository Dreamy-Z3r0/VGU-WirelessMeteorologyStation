#ifndef _TF_LOCALSTORAGE_H_
#define _TF_LOCALSTORAGE_H_

#include "Libraries.h"
#include "Macros_and_Defines.h"

#include "RTC.h"

#define FILENAME_SEPARATOR "-"
#define DIRECTORY_SEPARATOR "/"

#define SYSTEM_FILE "LOCAL-STORAGE.csv"

#define DAILY_LOG_EXTENSION ".csv"
#define DAILY_LOG_DIRECTORY "DAILY_LOG"

#define COMMUNICATION_LOG_EXTENSION ".txt"
#define COMMUNICATION_LOG_DIRECTORY "COMMUNICATION"

#define CHANGE_LOG_FILE "CHANGE-LOG.txt"


enum AccessDestination {SystemConfig, DailyLog, CommunicationLog, ChangeLog};


class Card_Access {
    public:
        Card_Access(uint32_t CSPin);

        void init();

        bool add_entry(AccessDestination writeTarget, String cardEntry);
        
        
    private:
        uint32_t CSPin;
        bool cardAvailable, calendarAvailable;

        int RTC_data[3];    // Year - Month - Day
        String Name_FromDate = "";

        bool Card_InUse = false;
        String Card_WriteBuffer = "";
  
        bool update_calendar(void); // Fetch calendar data to generate daily log file name
        void generate_filename(void);  // Generate a file name for daily log

        bool system_file_update(String cardEntry);
        bool analyse_header_tag(String headerTag);
};


#endif