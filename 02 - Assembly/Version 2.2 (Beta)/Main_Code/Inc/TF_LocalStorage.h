#ifndef _TF_LOCALSTORAGE_H_
#define _TF_LOCALSTORAGE_H_

#include "Libraries.h"
#include "Macros_and_Defines.h"

#include "RTC.h"

#define FILENAME_SEPARATOR "-"
#define FILENAME_EXTENSION ".csv"

class Card_Access {
    public:
        Card_Access(uint32_t CSPin);

        void init();

        bool create_file(String new_FileName);

    protected:
        int RTC_data[3];    // Year - Month - Day
        void update_calendar(void);

        String FileName = "";
        void generate_filename(void);

    private:
        uint32_t CSPin;
        bool cardAvailable;

        bool filename_check(String filename, String filename_extension = FILENAME_EXTENSION);

        
};


#endif