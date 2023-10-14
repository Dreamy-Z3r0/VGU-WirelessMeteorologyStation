/***
 *** LoRa connectivity test - Simple gateway
 *** 
 *** Microcontroller: ESP8266 on WeMos D1R2
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
 ***     ESP8266   | SX1278 |  Function
 *** --------------|--------|----------------
 ***       D7      |  MOSI  | VSPI MOSI
 ***       D6      |  MISO  | VSPI MISO
 ***       D5      |  SCK   | VSPI CLK
 ***       D8      |  NSS   | VSPI CS0
 *** --------------|--------|----------------
 ***       D1      |  RST   | LoRa Reset
 ***       D2      |  DIO0  | EXTI from LoRa
 ***/


#include <ESP8266WiFi.h>  // Wi-Fi connection handling library
#include "ThingSpeak.h"   // Library providing APIs for ThingSpeak access

#include <SPI.h>    // Built-in SPI library for LoRa
#include <LoRa.h>   // LoRa library handling SX1278

// Wi-Fi network credentials
const char* ssid = "Station - test network";   // SSID
const char* password = "testnetwork";          // Password

// ThingSpeak - Channel feed access parameters
unsigned long channelNumber = 2103173;
const char * writeAPIKey = "TKJB5M4KKRH868E4";

// LoRa module hardware dependencies
const int csPin = D8;      // SPI NCSS pin for LoRa
const int resetPin = D1;   // LoRa reset
const int irqPin = D2;     // Interrupt by LoRa

// LoRa setup parameters
const long frequency = 433E6;   // LoRa frequency

int spreadingFactor = 12;       // LoRa spreading factor
long signalBandwidth = 500E3;   // LoRa signal bandwidth
int codingRate4 = 8;            // Denominator for LoRa coding rate
int syncWord = 0x92;            // Sync word for LoRa communication

// Initiate a WiFiClient instance
WiFiClient client;

String message;
String rx_message = "";
bool nodeMessageAvailable = false;

void setup() {
  // Initiate Serial
  Serial.begin(9600);
  while (!Serial);

  WiFi.mode(WIFI_STA);
  ThingSpeak.begin(client);

  // Initialise LoRa
  LoRa.setPins(csPin, resetPin, irqPin);
  if (!LoRa.begin(frequency)) {
    Serial.println("LoRa init failed. Check your connections.");
    while (true);                       // if failed, do nothing
  }
  
  Serial.println("LoRa init succeeded.");

  // Set up LoRa parameters
  LoRaSettings();

  LoRa.onReceive(onReceive);
  LoRa.onTxDone(onTxDone);
  LoRa_rxMode();
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.print("Attempting to connect");
    while(WiFi.status() != WL_CONNECTED) {
      WiFi.begin(ssid, password); 
      delay(5000);     
    }
    Serial.println("\nConnected.");
  }
  
  if (nodeMessageAvailable) {
    nodeMessageAvailable = false;
    
    int packetRSSI = LoRa.packetRssi();   // dBm
    float packetSNR = LoRa.packetSnr();   // dB
    long packetFreqError = LoRa.packetFrequencyError();   // Hz

    float latitude, longtitude;
    int commaIndex = rx_message.indexOf(',');
    latitude = rx_message.substring(0, commaIndex).toFloat();
    longtitude = rx_message.substring(commaIndex+1).toFloat();

    Serial.println("(Latitude, Longtitude) = (" + rx_message + ")");
    Serial.println("RSSI = " + String(packetRSSI) + " dBi");
    Serial.println("SNR = " + String(packetSNR) + " dB");
    Serial.println("Frequency error = " + String(packetFreqError) + " Hz");

    ThingSpeak.setField(1, packetRSSI);
    ThingSpeak.setField(2, packetSNR);
    ThingSpeak.setField(3, packetFreqError);
    ThingSpeak.setField(4, latitude);
    ThingSpeak.setField(5, longtitude);
    ThingSpeak.setLatitude(latitude);
    ThingSpeak.setLongitude(longtitude);

    int returnedCode = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
    LoRa_sendMessage("done");
  }
}

void LoRaSettings(void) {
  LoRa.setSpreadingFactor(spreadingFactor);
  LoRa.setSignalBandwidth(signalBandwidth);
  LoRa.setCodingRate4(codingRate4);
  LoRa.setSyncWord(syncWord);
}

// Set LoRa in receive mode
void LoRa_rxMode() {
  LoRa.disableInvertIQ();   // normal mode
  LoRa.receive();           // set receive mode
}

// Set LoRa in transmit mode
void LoRa_txMode() {
  LoRa.idle();              // set standby mode
  LoRa.enableInvertIQ();    // active invert I and Q signals
}

void LoRa_sendMessage(String message) {
  LoRa_txMode();                        // set tx mode
  LoRa.beginPacket();                   // start packet
  LoRa.print(message);                  // add payload
  LoRa.endPacket(true);                 // finish packet and send it
}

void onReceive(int packetSize) {
  rx_message = "";

  while (LoRa.available()) {
    rx_message += (char)LoRa.read();
  }

  nodeMessageAvailable = true;
}

void onTxDone() {
  Serial.println("TxDone\n");
  LoRa_rxMode();
}
