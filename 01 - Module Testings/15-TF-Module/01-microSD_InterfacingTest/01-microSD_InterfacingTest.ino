#include "SPI.h"
#include "SD.h"

#define CSPin PA4
File cardData;

void setup() {
     pinMode(CSPin, OUTPUT);
     if (!SD.begin(CSPin)) {
          return;     // do nothing more
     } else {
      delay(1000);
     }

     String testString = "This is a test.";
     int testInt = 255;
     float testFloat = 125.3;

     cardData = SD.open("data.txt", FILE_WRITE);
     if (cardData) {
          cardData.println(testString);     
          cardData.println(testInt, HEX);     
          cardData.println(testFloat);
          cardData.close();
     }
}

void loop() {
     // no need for a loop
}
