/***
 *** LoRa connectivity test - Simple node
 ***
 *** Microcontroller: STM32F103CBT6 @ 72MHz with remapped SPI
 ***    + U(S)ART support: "Enabled (generic 'Serial')"
 ***    + USB support (if available): "None"    
 ***    + USB speed (if available): "Low/Full Speed"
 ***    + Optimize: "Smallest (-Os default)"
 ***    + Debug symbols: "None"
 ***    + C Runtime Library: "Newlib Nano + Float Printf"    
 ***    
 *** LoRa module: SX1278
 *** LoRa settings are input via Serial and updated with LoRaSettings(..)
 *** Initial LoRa settings:
 ***    + LoRa frequency: 433 MHz
 ***    + Spreading factor: 12    
 ***    + Signal bandwidth: 500 kHz    
 ***    + Coding rate: 4/5 
 ***    + Sync word: 0x92     
 *** 
 ***      Hardware connections
 ***  STM32  | SX1278 |  Function
 *** --------|--------|----------------
 ***   PB15  |  MOSI  | MOSI
 ***   PB14  |  MISO  | MISO
 ***   PB13  |  SCK   | CLK
 ***   PB12  |  NSS   | CS0
 *** --------|--------|----------------
 ***   PA8   |  RST   | LoRa Reset
 ***   PA11  |  DIO0  | EXTI from LoRa
 */


#include <SPI.h>
#include <LoRa.h>

#define SPI2_MOSI_Pin PB15   // SPI2 MOSI pin
#define SPI2_MISO_Pin PB14   // SPI2 MISO pin
#define SPI2_SCLK_Pin PB13   // SPI2 SCLK pin

const long frequency = 433E6;   // LoRa frequency

const int csPin = PB12;    // SPI NCSS for LoRa
const int resetPin = PA8;  // LoRa reset
const int irqPin = PA11;   // Interrupt by LoRa

int spreadingFactor = 12;
long signalBandwidth = 500E3;
int codingRate4 = 8;
int syncWord = 0x92;

String gateway_msg;
bool gatewayMessaged = false;

int rx_packetRssi;
float rx_packetSnr;
long rx_packetFrequencyError;

int globalIndex = 0;
String SerialMessage = "";
bool SerialMessageAvailable = false;

uint32_t LED_INDICATOR = PC13;
unsigned long timestamp;

void setup() {
  pinMode(LED_INDICATOR, OUTPUT);
  digitalWrite(LED_INDICATOR, LOW);
  
  // Initialize Serial
  Serial.begin(9600);
  while (!Serial);
  delay(2000);

  // Initialize LoRa
  SPIClass* NewSPI = new SPIClass(SPI2_MOSI_Pin, SPI2_MISO_Pin, SPI2_SCLK_Pin);
  LoRa.setSPI(*NewSPI);
  LoRa.setPins(csPin, resetPin, irqPin);
  if (!LoRa.begin(frequency)) {
    Serial.println("LoRa init failed. Check your connections.");
    while (!LoRa.begin(frequency)) {
      Serial.println("LoRa init failed. Check your connections.");
      delay(4000);                   
    }
  }

  // Set up LoRa parameters
  LoRaSettings();

  LoRa.onReceive(onReceive);
  LoRa.onTxDone(onTxDone);
  LoRa_rxMode();
}

void loop() {
  Serial_InputHandler();
  if (SerialMessageAvailable) {
    SerialMessageAvailable = false;
    digitalWrite(LED_INDICATOR, HIGH);

    Serial.printf("Index: %d\n", globalIndex);
    
    int commaSeparator = SerialMessage.indexOf(',');
    Serial.printf("  Latitude = %s; ", SerialMessage.substring(0, commaSeparator).c_str());
    Serial.printf("Longtitude = %s \n", SerialMessage.substring(commaSeparator+1).c_str());

//    int attempt = 1;
//    Serial.printf("  Sending... Attempt %d\n", attempt++);
    LoRa_sendMessage(SerialMessage);
//    globalIndex += 1;

//    timestamp = millis();
//    while (!gatewayMessaged) {
//      if (millis() - timestamp >= 10000) {
//        timestamp = millis();
////        Serial.printf("  Sending... Attempt %d\n", attempt++);
//        LoRa_sendMessage(SerialMessage);
//      }
//    }
  }

//  if (gatewayMessaged) {
//    gatewayMessaged = false;
//    digitalWrite(LED_INDICATOR, LOW);
//    Serial.println("Received: " + gateway_msg);
//
//    if (gateway_msg.equals("done")) {
//      Serial.printf("   RSSI = %d dBm\n", rx_packetRssi);
//      Serial.printf("   SNR = %d dB\n", rx_packetSnr);
//      Serial.printf("   Frequency error = %d Hz\n\n", rx_packetFrequencyError);
//    }
//  }
}

void LoRaSettings(void) {
  LoRa.setSpreadingFactor(spreadingFactor);
  LoRa.setSignalBandwidth(signalBandwidth);
  LoRa.setCodingRate4(codingRate4);
  LoRa.setSyncWord(syncWord);
}

void LoRa_rxMode(){
  LoRa.enableInvertIQ();                // active invert I and Q signals
  LoRa.receive();                       // set receive mode
}

void LoRa_txMode(){
  LoRa.idle();                          // set standby mode
  LoRa.disableInvertIQ();               // normal mode
}

void LoRa_sendMessage(String message) {
  LoRa_txMode();                        // set tx mode
  LoRa.beginPacket();                   // start packet
  LoRa.print(message);                  // add payload
  LoRa.endPacket(true);                 // finish packet and send it
}

void onReceive(int packetSize) {
  gateway_msg = "";

  while (LoRa.available()) {
    gateway_msg += (char)LoRa.read();
  }

  rx_packetRssi = LoRa.packetRssi();
  rx_packetSnr = LoRa.packetSnr();
  rx_packetFrequencyError = LoRa.packetFrequencyError();

  gatewayMessaged = true;
}

void onTxDone() {
  LoRa_rxMode();
  Serial.printf("  Coordinations sent.\n");
}

void Serial_InputHandler(void) {
  if (Serial.available()) {
    SerialMessage = Serial.readStringUntil('\n');    
    SerialMessageAvailable = true;
  }
}
