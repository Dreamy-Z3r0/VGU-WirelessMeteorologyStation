#include "../Inc/TF_LocalStorage.h"


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

void create_file(String new_FileName) {

}

void Card_Access::update_calendar(void) {
    RTC_data[0] = RTC_DS3231.readYear();
    RTC_data[1] = RTC_DS3231.readMonth();
    RTC_data[2] = RTC_DS3231.readDay();
}

void Card_Access::generate_filename(void) {
    String FileName_temp = "";

    update_calendar();

    FileName_temp += String(RTC_data[0]);
    FileName_temp += FILENAME_SEPARATOR;

    if (10 > RTC_data[1]) FileName_temp += "0";
    FileName_temp += String(RTC_data[1]);
    FileName_temp += FILENAME_SEPARATOR;

    if (10 > RTC_data[2]) FileName_temp += "0";
    FileName_temp += String(RTC_data[2]);

    FileName_temp += FILENAME_EXTENSION;

    if (!FileName.equals(FileName_temp)) {
        FileName = FileName_temp;
    }
}

bool Card_Access::filename_check(String filename, String filename_extension) {
    // Look for the index of the filename extension
    int extensionIndex = filename.lastIndexOf('.');

    if (-1 == extensionIndex) {
        return false;   // Filename does not contain an extension
    } else {
        // Extract filename extension
        String extractedExtension = filename.substring(extensionIndex);

        // Return "true" if the extension is valid (ignore case)
        return extractedExtension.equalsIgnoreCase(filename_extension);
    }
}