// Comment/Uncomment to disable/enable printing on Serial Monitor
// #define SERIAL_MONITOR_OUTPUT   
// #define BAUD_RATE 9600


/* Library dependencies */

#include <ESP8266WiFi.h>  // Wi-Fi connection handling library
#include "ThingSpeak.h"   // Library providing APIs for ThingSpeak access

#include <SPI.h>    // Built-in SPI library, used for LoRa
#include <LoRa.h>   // LoRa library handling SX1278


/* Setup parameters */

// Wi-Fi network credentials
const char* ssid = "Station - test network";   // SSID
const char* password = "testnetwork";          // Password

// ThingSpeak - Channel feed access parameters
unsigned long channelNumber = 2103173;
const char* writeAPIKey = "TKJB5M4KKRH868E4";

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

// LoRa message: field separator
char delimiter = ',';


/* Objects utilising included libraries and/or user-defined modules */

// Initiate a WiFiClient instance
WiFiClient client;


/* Runtime macros and variables */
String rx_message = "";   // Buffer storing received messages via LoRa
bool nodeMessageAvailable = false;    // LoRa received message status


void setup() {
  #ifdef SERIAL_MONITOR_OUTPUT
  Serial.begin(BAUD_RATE);
  #endif
  
  // Put ESP8266 in station mode
  WiFi.mode(WIFI_STA);

  // Initiate ThingSpeak driver with the WiFiClient instance
  ThingSpeak.begin(client);

  // Pass the corresponding hardware connections to LoRa software driver
  LoRa.setPins(csPin, resetPin, irqPin);

  // Initiate the software driver of the LoRa hardware module
  int x;  // Stores return code of the initialisation operation of hardware module
  do {
    x = LoRa.begin(frequency);
  } while (0 == x);   // The software of the gateway does not proceed
                      // if the LoRa hardware module is not initialised.

  // Push the LoRa parameters to the hardware module
  LoRa_parameter_setup();

  // Pass the function definitions of .onReceive and .onTxDone to the corresponding LoRa driver's methods
  LoRa.onReceive(onReceive);
  LoRa.onTxDone(onTxDone);

  // Put the gateway in receive mode initially
  LoRa_rxMode();
}

void loop() {
  // Make sure the gateway has an Internet connection to push data to ThingSpeak server
  while (WiFi.status() != WL_CONNECTED) {
    WiFi.begin(ssid, password);
    delay(5000);
  }

  if (nodeMessageAvailable) {
    nodeMessageAvailable = false;   // Clear status flag
    LoRa_to_ThingSpeak();           // Parse LoRa message and update the corresponding data fields of ThingSpeak upload buffer

    // Upload data to ThingSpeak
    int x = ThingSpeak.writeFields(channelNumber, writeAPIKey);   

    #ifdef SERIAL_MONITOR_OUTPUT
    if (200 == x) {   // Data is uploaded
      Serial.println("Data uploaded to ThingSpeak.");
    } else {          // Display error code + corresponding error
      Serial.print("Error " + String(x) + ": ");
      switch (x) {
        case 404: {
          Serial.println("Incorrect API key (or invalid ThingSpeak server address)");
          break;
        }
        case -301: {
          Serial.println("Failed to connect to ThingSpeak");
          break;
        }
        case -302: {
          Serial.println("Unexpected failure during write to ThingSpeak");
          break;
        }
        case -303: {
          Serial.println("Unable to parse response");
          break;
        }
        case -303: {
          Serial.println("Timeout waiting for server to respond");
          break;
        }
        case -401: {
          Serial.println("Point was not inserted (most probable cause is the rate limit of once every 15 seconds)");
          break;
        }
        case 0: {
          Serial.println("Other error");
          break;
        }
        default: {
          Serial.println("Unknown error");
          break;
        }
      }
    }
    #endif
  }
}

void LoRa_parameter_setup() {
  LoRa.setSpreadingFactor(spreadingFactor);
  LoRa.setSignalBandwidth(signalBandwidth);
  LoRa.setCodingRate4(codingRate4);
  LoRa.setSyncWord(syncWord);
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

// Parsing LoRa message for ThingSpeak
void LoRa_to_ThingSpeak() {
  int delimiterIndex = rx_message.indexOf(delimiter);
  ThingSpeak.setCreatedAt(rx_message.substring(0, delimiterIndex));

  for (int fieldIndex = 1; fieldIndex <= 7; fieldIndex += 1) {
    float fieldValue;

    int startPoint = delimiterIndex + 1;
    if (fieldIndex < 7) {
      delimiterIndex = rx_message.indexOf(delimiter, startPoint);
      fieldValue = rx_message.substring(startPoint, delimiterIndex).toFloat();
    } else {
      fieldValue = rx_message.substring(startPoint).toFloat();
    }

    ThingSpeak.setField(fieldIndex, fieldValue);
  }
}
