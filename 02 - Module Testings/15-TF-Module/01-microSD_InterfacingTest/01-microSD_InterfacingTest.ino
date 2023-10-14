#include "SPI.h"
#include "SD.h"

#define CSPin PA4
File cardData;

#define LED_ONBOARD PC13
bool LED_State = true;

void setup() {
  delay(2000);

  Serial.begin(9600);
  pinMode(LED_ONBOARD, OUTPUT);

  TIM_TypeDef *Instance = TIM1;
  HardwareTimer *MsgTim = new HardwareTimer(Instance);
  MsgTim->setOverflow(1, HERTZ_FORMAT);
  MsgTim->attachInterrupt(BlinkRoutine);
  MsgTim->resume();
  
  pinMode(CSPin, OUTPUT);
  if (!SD.begin(CSPin)) {
    return;     // do nothing more
  } else {
    delay(1000);
  }

  String testString = "This is a test.";
  int testInt = 255;
  float testFloat = 125.3;

  if (!SD.exists("folder")) {
    SD.mkdir("folder");
  } else if (SD.exists("folder/data.csv")) {
    SD.remove("folder/data.csv");
  }
  
  cardData = SD.open("folder/data.csv", FILE_WRITE);
  if (cardData) {
    cardData.println(testString);
    cardData.print("testInt,");
    cardData.println(testInt);
    cardData.print("testInt (HEX),");
    cardData.println(testInt, HEX);
    cardData.print("testFloat,");
    cardData.println(testFloat);
    cardData.print("testFloat (4th decimal),");
    cardData.println(testFloat, 4);

    cardData.close();
  }
}

void loop() {
  cardData = SD.open("folder/data.csv");
  if (cardData) {
    while (cardData.available()) {
      Serial.write(cardData.read());
    }

    cardData.close();
    while(1);
  } else {
    Serial.println("Error openning card.");
  }
}

void BlinkRoutine(void) {
  digitalWrite(LED_ONBOARD, LED_State);
  LED_State = !LED_State;
}
