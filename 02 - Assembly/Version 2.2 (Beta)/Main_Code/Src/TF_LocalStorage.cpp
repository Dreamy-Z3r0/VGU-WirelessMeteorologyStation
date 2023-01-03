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

bool create_file(String new_FileName) {
    bool isFilenameValid = filename_check(new_FileName);

    if (!isFilenameValid) {
        return false;
    } else {
        int DirectoryStartIndex = new_FileName.indexOf('/'),
            DirectoryEndIndex = new_FileName.lastIndexOf('/');

        if (DirectoryStartIndex == DirectoryEndIndex) {
            if (0 == DirectoryStartIndex) {
                newFileName = new_FileName.substring(1);
            } else if (0 < DirectoryStartIndex) {
                SD.mkdir(new_FileName.substring(0, DirectoryStartIndex));
                new_FileName = new_FileName.substring(DirectoryStartIndex+1);
            }
        } else {
            if (0 == DirectoryStartIndex) {
                SD.mkdir(new_FileName.substring(DirectoryStartIndex+1, DirectoryEndIndex));
                new_FileName = new_FileName.substring(DirectoryEndIndex+1);
            } else {
                SD.mkdir(new_FileName.substring(0, DirectoryEndIndex));
            }
        }
    }
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