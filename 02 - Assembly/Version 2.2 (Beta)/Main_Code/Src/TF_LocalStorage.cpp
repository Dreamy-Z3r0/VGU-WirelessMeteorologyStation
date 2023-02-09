#include "../Inc/TF_LocalStorage.h"

File cardData;

Card_Access::Card_Access(uint32_t CSPin) {
    this->CSPin = CSPin;
    cardAvailable = false;
}

void Card_Access::init() {
    pinMode(CSPin, OUTPUT);
    if (!SD.begin(CSPin)) {
        cardAvailable = false;
    } else {
        cardAvailable = true;
    }
}

bool Card_Access::log_entry(String data) {
    create_file();

    cardData = SD.open(FileName, FILE_WRITE);
    if (cardData) {
        cardData.println(data);
        cardData.close();
    } else {
        return false;
    }

    return true;
}

// Create a daily log file
bool Card_Access::create_daily_log(void) {
    if (cardAvailable) {
        calendarAvailable = update_calendar();
        generate_dailylog_filename();

        cardData = SD.open(DailyLog_Name, FILE_WRITE);
        if (cardData) {
            String firstRow_Meteorological = "Time,Rainfall amount,Wind speed,Wind direction,";
            String firstRow_Temperature = "Ambient temperature (ground),Ambient temprature (1m),";
            String firstRow_Unverifiable = "Relative humidity,Barometric pressure";

            cardData.print(firstRow_Meteorological);
            cardData.print(firstRow_Temperature);
            cardData.println(firstRow_Unverifiable);

            cardData.close();
        } else {
            return false;
        }
    } else {
        return false;
    }

    return true;
}

bool Card_Access::update_calendar(void) {
    // Update the calendar
    RTC_data[0] = RTC_DS3231.readYear();
    RTC_data[1] = RTC_DS3231.readMonth();
    RTC_data[2] = RTC_DS3231.readDay();

    // Check and print the validity of RTC data
    DS3231 RTC_quality;
    return RTC_quality.oscillatorCheck();
}

// Generate a file name in "YYYY-MM-DD.csv" format
void Card_Access::generate_dailylog_filename(void) {     
    String FileName_temp = "";      // Create a temporatory holder for daily log name

    FileName_temp += String(RTC_data[0]);       // "YYYY-"; no need to check for number of 0s
    FileName_temp += FILENAME_SEPARATOR;        // since years counted by DS3231 go from 1970

    if (10 > RTC_data[1]) FileName_temp += "0";     // "MM-"; adding a '0' for months
    FileName_temp += String(RTC_data[1]);           // from January to September
    FileName_temp += FILENAME_SEPARATOR;

    if (10 > RTC_data[2]) FileName_temp += "0";     // "DD"; adding a '0' for days
    FileName_temp += String(RTC_data[2]);           // from 1 to 9

    FileName_temp += FILENAME_EXTENSION;    // Compulsary file name extension

    if (!DailyLog_Name.equals(FileName_temp)) {     // Double check file name for repeat
        DailyLog_Name = FileName_temp;
    }
}