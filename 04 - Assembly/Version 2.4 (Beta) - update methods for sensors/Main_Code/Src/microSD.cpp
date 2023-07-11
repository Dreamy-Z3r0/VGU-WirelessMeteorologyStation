#include "../Inc/microSD.h"

File cardData;


/**********************
 *** Constructor(s) ***
 **********************/
Card_Access::Card_Access(uint32_t CSPin) {
    this->CSPin = CSPin;
    cardAvailable = false;
}


/**********************
 *** Initialization ***
 **********************/

void Card_Access::init() {
    pinMode(CSPin, OUTPUT);
    if (!SD.begin(CSPin)) {
        cardAvailable = false;
    } else {
        cardAvailable = true;
    }
}


bool Card_Access::add_entry(AccessDestination writeTarget, String cardEntry) {
    if (cardAvailable) {
        if (Card_InUse) {
            return false;
        } else {
            Card_InUse = true;

            // Update calendar, then file name by date for daily log and communication log
            calendarAvailable = update_calendar();
            if (!calendarAvailable) {
                Card_InUse = false;
                return false;
            } else {
                generate_filename();
            }

            // Generate targeted file directory
            String FileToWrite = "";
            switch (writeTarget) {
                case SystemConfig: { // "LOCAL-STORAGE.csv"
                    FileToWrite = SYSTEM_FILE;
                    break;
                }
                case DailyLog: { // "DAILY_LOG/YYYY-MM-DD.csv"
                    FileToWrite  = DAILY_LOG_DIRECTORY;
                    FileToWrite += DIRECTORY_SEPARATOR;
                    FileToWrite += Name_FromDate;
                    FileToWrite += DAILY_LOG_EXTENSION;

                    if (!SD.exists(FileToWrite)) {
                        cardData = SD.open(FileToWrite, FILE_WRITE);
                        if (cardData) {     // Add first row of column identifiers
                            String firstRow_Meteorological = "Time,Rainfall amount,Wind speed,Wind direction,";
                            String firstRow_Temperature = "Ambient temperature (ground),Ambient temprature (1m),";
                            String firstRow_Unverifiable = "Relative humidity,Barometric pressure";

                            cardData.print(firstRow_Meteorological);
                            cardData.print(firstRow_Temperature);
                            cardData.println(firstRow_Unverifiable);

                            cardData.close();
                        } else {            // Error accessing the microSD card
                            Card_InUse = false;
                            return false;
                        }
                    }
                    break;
                }
                case CommunicationLog: { // "COMMUNICATION/YYYY-MM-DD.csv"
                    FileToWrite += COMMUNICATION_LOG_DIRECTORY;
                    FileToWrite += DIRECTORY_SEPARATOR;
                    FileToWrite += Name_FromDate;
                    FileToWrite += COMMUNICATION_LOG_EXTENSION;
                    break;
                }
                case ChangeLog: { // "CHANGE-LOG.txt"
                    FileToWrite += SYSTEM_FILE;
                    break;
                }
                default: {  // Unknown target
                            // never the case due to invalid input parameter
                    Card_InUse = false;
                    return false;
                }
            }

            // Open file; or create file, if it doesn't exist
            if (SystemConfig != writeTarget) {
                cardData = SD.open(FileToWrite, FILE_WRITE);
                if (cardData) {
                    cardData.println(cardEntry);
                    cardData.close();
                } else {
                    Card_InUse = false;
                    return false;
                }
            } else {
                // Update system configuration file
            }

            Card_InUse = false;
            return true;
        }
    }
}

// Supporting method: Fetch calendar data to generate file name
bool Card_Access::update_calendar(void) {
    // Update the calendar
    RTC_data[0] = RTC_DS3231.readYear();
    RTC_data[1] = RTC_DS3231.readMonth();
    RTC_data[2] = RTC_DS3231.readDay();

    // Check and print the validity of RTC data
    DS3231 RTC_quality;
    return RTC_quality.oscillatorCheck();
}

// Generate a file name by date in YYYY-MM-DD format
void Card_Access::generate_filename(void) {     
    String FileName_temp = "";      // Create a temporatory holder for file name by date

    FileName_temp += String(RTC_data[0]);       // "YYYY-"; no need to check for number of 0s
    FileName_temp += FILENAME_SEPARATOR;        // since years counted by DS3231 go from 1970

    if (10 > RTC_data[1]) FileName_temp += "0";     // "MM-"; adding a '0' for months
    FileName_temp += String(RTC_data[1]);           // from January ("01") to September ("09")
    FileName_temp += FILENAME_SEPARATOR;

    if (10 > RTC_data[2]) FileName_temp += "0";     // "DD"; adding a '0' for days
    FileName_temp += String(RTC_data[2]);           // from 1 to 9

    if (!Name_FromDate.equals(FileName_temp)) {     // Double check file name for repeat
        Name_FromDate = FileName_temp;
    }
}